
/********************************************
*			STM32F439 Main (C Startup File)  			*
*			Developed for the STM32								*
*			Author: 						*
*			Source File														*
*     Updated: 04/03/2026 	  							*
********************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "main.h"

#define OUTGOING_PACKET_LENGTH_BYTES 5
#define INCOMING_PACKET_LENGTH_BYTES 2

uint16_t sample_ADC();
float adc_to_temp(uint16_t value);
void debounce(struct Button *button, uint8_t input);
uint8_t* construct_packet(void);
void send_data(void);
int getPacket(void);

struct Button fanButton;			//global button structs
struct Button lightButton;

bool isHeaterOn = false;
bool isCoolingOn = false;
volatile double currentTemp = 0;
volatile uint32_t ms_counter = 0;

bool autoControl = true;													// True - heater/cooler/fan controlled by temp, False - controlled by UART
bool fanControl = false;													// True - Fan in manual off control, False - Fan in auto control
uint32_t auto_lockout = 0;												// Timer for counting lockout time till auto override - max 10s
uint32_t fan_lockout = 0;													// Timer for counting fan lockout time till auto override - max 10s
void fanLightLogic(uint8_t lightSensor);					// Handle fan and light switch logic
void tempControl();																// Handle setting cooler/heater	based on temperature				
void ledControl();																// Control LEDs based on current system settings



//******************************************************************************//
// Function: main()
// Input : None
// Return : None
// Description : Entry point into the application.
// *****************************************************************************//
int main(void)
{
	// Bring up the GPIO for the power regulators.
	boardSupport_init();
	// Peripheral config
	RCC_init();
	LED_GPIO_config();
	Input_GPIO_config();
	ADC_config();
	UART_config();

	// Timer6 interrupt config
	__disable_irq();												// Disable global interrupt system
	timer6_config();												// Timer6 config
	NVIC_SetPriority(TIM6_DAC_IRQn, 2);			// Set timer interrupt priority to 2
	NVIC_EnableIRQ(TIM6_DAC_IRQn);					// Enable interrupt
	__enable_irq();													// Enable global interrupt system
	TIM6->CR1 |= TIM_CR1_CEN;								// Enable Timer6
	
	fanButton.output = 1;										// Set fan to be on by default

  while (1)
  {
		// Read ADC
		currentTemp = adc_to_temp(sample_ADC());

		// Poll Switches
		volatile uint8_t fanInput = GPIOB->IDR & GPIO_IDR_ID0;				//Fan switch - PB0
		volatile uint8_t lightInput = GPIOA->IDR & GPIO_IDR_ID10;		  //Light switch - PA10
		
		if (autoControl == true)
		{
			// Allow fan button and light button to work when in auto control
			debounce(&fanButton, fanInput);
			debounce(&lightButton, lightInput);
		}
		else if ((autoControl == false) && (auto_lockout >= 1000))
		{
			// If not in auto control only let light button work if 1 sec passed else UART takes precedence
			debounce(&lightButton, lightInput);
		}
		

  	// Send data to PC at 0.25 Hz
		if (ms_counter >= 4000) // If 4000ms has elapsed, send data to PC
		{
			ms_counter = 0;
			send_data();
		}

		// Do stuff based on light and fan switch state
		volatile uint8_t lightIntensity = GPIOA->IDR & GPIO_IDR_ID8;	// Light intensity sensor - PA8
																																	// Maybe add hold functionality later
		fanLightLogic(lightIntensity);

		// Check UART
		if(getPacket() == 1)
		{
			// If successful packet received switch to UART controlled mode
			autoControl = false;
			auto_lockout = 0;
			if ((isCoolingOn == true) && (isHeaterOn == true))
			{
				// If both cooling and heating set on by UART default to only cooling on
				isCoolingOn = true;
				isHeaterOn = false;
			}
		}
		
		// Once 10s lockout complete switch back to auto control
		if (autoControl == false && auto_lockout >= 10000)	
		{
			autoControl = true;
		}

		// Temp control - control fan + heater setting based on temp only if in auto control
		tempControl();

		// Change LEDs based on settings
		ledControl();

		// Loop


  }
}

// TIM6 1 ms interrupt handler
void TIM6_DAC_IRQHandler()
{
	//Clear timer interrupt flag
	TIM6->SR &= ~TIM_SR_UIF;

	// Do interrupt logic
	// Increment button timers
	fanButton.hold_time++;
	fanButton.lockout_time++;
	lightButton.hold_time++;
	lightButton.lockout_time++;
	
	//Increment lockout timers
	fan_lockout++;
	auto_lockout++;
	
	// Increment timer for UART packet send - 0.25 Hz
	ms_counter++;
	
	// No need to restart timer as it is in free-run mode
}

void debounce(struct Button *button, uint8_t input)
{
	switch (button->state)
	{
		case NO_INPUT:
			// Detect falling edge - active low
			if (button->prevInput == 1 && input == 0)
			{
				// If button pressed reset timer and switch to pressed mode
				button->hold_time = 0;
				button->state = PRESSED;
			}

			break;

		case PRESSED:
			// Detect rising edge
			if (input == 1)
			{
				if (button->hold_time >= 10)
				{
					// If button has been released and 10 ms have passed change state
					button->state = CONFIRM;
				}
				else
				{
					// If button is released and not enough time then disregard result
					button->state = NO_INPUT;
				}
			}
			break;

		case CONFIRM:
			// Reset lockout timer
			button->lockout_time = 0;
			button->state = LOCKOUT;

			// Flip button output
			if (button->output == 0)
			{
				button->output = 1;
			}
			else if (button->output == 1)
			{
				button->output = 0;
			}
			break;

		case LOCKOUT:
			// No input for 2s
			if (button->lockout_time >=2000)
			{
				button->state = NO_INPUT;
			}
			break;
	}
	button->prevInput = input;

}

// Reads current ADC value
uint16_t sample_ADC()
{
	uint16_t current_ADC = 0;

	// Trigger and ADC conversion;
	ADC3->CR2 |= ADC_CR2_SWSTART;

	// Wait for conversion to complete
	while((ADC3->SR & ADC_SR_EOC) == 0x00);

	// Get value from ADC
	current_ADC = (ADC3->DR & 0x0000FFFF);
	return current_ADC;
}

// Converts ADC value to temp value
float adc_to_temp(uint16_t value)
{
	//return (55 - ((float)value)*(85/4095));
	return (55 - ((float)value)*(0.02075702076));
}

uint8_t* construct_packet(void)
{
	static uint8_t packet[OUTGOING_PACKET_LENGTH_BYTES];
	uint8_t index = 0;
	packet[index++] = 0x26;
	packet[index++] = 0x7E;

	// Choose the sign for the temperature
	char sign = '+';
	if (currentTemp < 0)
	{
		sign = '-';
	}

	// Get the absolute value of the temperature
	double absTemp = currentTemp;
	if (currentTemp < 0)
	{
		absTemp = -currentTemp;
	}

	char tempBuffer[8]; // Set up temporary buffer for temperature string
	snprintf(tempBuffer, sizeof(tempBuffer), "%c%05.2f", sign, absTemp);

	// Copy temperature string into packet
	for (uint8_t i = 0; i < 8; i++) {
		packet[index++] = (uint8_t)tempBuffer[i];
	}

	packet[index++] = 0x7E; // tilde delimiter

	packet[index++] = 0x00; // 0
	packet[index++] = 0x01; // 1
	packet[index++] = (uint8_t)((bool)lightButton.output); // 1 is light is on, 0 otherwise
	packet[index++] = (uint8_t)(isHeaterOn); // 1 is heater is on, 0 otherwise
	packet[index++] = (uint8_t)(isCoolingOn); // 1 is cooling is on, 0 otherwise
	packet[index++] = (uint8_t)((bool)fanButton.output);// 1 is fan is on, 0 otherwise
	packet[index++] = 0x00; // 0
	packet[index] = 0x01; // 1

	return packet;
}

void send_data()
{
	uint8_t* packet = construct_packet();

	for (uint8_t i = 0; i < OUTGOING_PACKET_LENGTH_BYTES; i++) {
		uint32_t timeout = 10000; // idk if this is the right timeout value
		while ((USART3->SR & USART_SR_TXE) == 0 && timeout > 0) {
			timeout--;
		}
		if (timeout > 0) {
			USART3->DR = packet[i];
		}
	}
}

int8_t getByte(void)
{
	int8_t receivedByte = -1;

	if (USART3->SR & USART_SR_RXNE)
	{
		receivedByte = USART3->DR;
	}

	return receivedByte;
}

int getPacket(void)
{
	static uint8_t state = 0; // 0: waiting for 0x26, 1: waiting for control byte
	int8_t receivedByte = getByte();

	if (receivedByte != -1)
	{
		if (state == 0)
		{
			if (receivedByte == 0x26)
			{
				state = 1;
			}
		}
		else if (state == 1)
		{
			uint8_t controlByte = (uint8_t)receivedByte;

			// Control byte format is 01abcd10
			// Bits 7-6 must be 01, and bits 1-0 must be 10.
			if ((controlByte & 0xC3) == 0x42) // 0xC3 = 0b11000011, 0x42 = 0b01000010
			{
				if ((currentTemp >= 15) || (currentTemp <= 30))
				{
					// UART input only valid if temp between 15 and 30
					lightButton.output = (controlByte >> 5) & 0x01; // a - bit 5: Light Output
					isHeaterOn = (controlByte >> 4) & 0x01; // b - bit 4: Heater Output
					isCoolingOn = (controlByte >> 3) & 0x01; // c - bit 3: Cooling Output
					fanButton.output = (controlByte >> 2) & 0x01; // d - bit 2: Fan Output

					state = 0;
					return 1; // packet successfully processed
				}
				else
				{
					// discard packet
					state = 0;
				}
			}
			else
			{
				// discard packet
				state = 0;
			}
		}
	}

	return -1;
}

void fanLightLogic(uint8_t lightSensor)
{
	// Fan control
	if (fanButton.output == 0)	// Fan is switched off
	{
		if (fanControl == false)
		{
			// If fan has been switched off for the first time enable 10s countdown and manual control
			fan_lockout = 0;
			fanControl = true;
		}
		else if (fanControl == true)	// Fan is in manual control
		{
			if (fan_lockout >= 10000)
			{
				// If 10s passed turn fan back on and turn off manual control
				fanControl = false;
				fanButton.output = 1;
			}
		}
	}
	else
	{
		fanControl = false;
	}
	
	// Light control
	if ((lightButton.output == 1) && (lightSensor == 1))
	{
		// If light button is pressed and light sensor detects light then switch off light button
		lightButton.output = 0;
	}
	else if ((lightButton.output == 1) && (lightSensor == 0))
	{
		// If light button is pressed and no light detected then allow button input to continue
		lightButton.output = 1;
	}
	
}

void tempControl()
{
	if (autoControl == true)
	{
		if (fanControl == false)	// If fan off manual overridde is not on
		{
			if (currentTemp < 22)
			{
				// If temp less then 22, heater + fan ON
				isHeaterOn = true;
				isCoolingOn = false;
				fanButton.output = 1;
			}
			else if (currentTemp > 24)
			{
				// If temp more then 24, cooler + fan ON
				isHeaterOn = false;
				isCoolingOn = true;
				fanButton.output = 1;
			}
			else
			{
				// Else in hysteresis band, heater + cooler OFF, fan ON
				isHeaterOn = false;
				isCoolingOn = false;
				fanButton.output = 1;
			}
		}
		else if (fanControl == true)	// If fan off manual overridde is on
		{
			if (currentTemp < 22)
			{
				// If temp less then 22, heater ON + fan OFF
				isHeaterOn = true;
				isCoolingOn = false;
				fanButton.output = 0;
			}
			else if (currentTemp > 24)
			{
				// If temp more then 24, cooler ON + fan OFF
				isHeaterOn = false;
				isCoolingOn = true;
				fanButton.output = 0;
			}
			else
			{
				// Else in hysteresis band, heater + cooler OFF, fan OFF
				isHeaterOn = false;
				isCoolingOn = false;
				fanButton.output = 0;
			}
		}
	}
}

void ledControl()
{
	if (fanButton.output == 1)	// Fan control output - PB1
	{
		GPIOB->ODR &= ~(GPIO_ODR_OD1);	// Active low
	}
	else
	{
		GPIOB->ODR |= GPIO_ODR_OD1;
	}
	
	if (lightButton.output == 1)	// Light control output - PA9
	{
		GPIOA->ODR &= ~(GPIO_ODR_OD9);	// Active low
	}
	else
	{
		GPIOA->ODR |= GPIO_ODR_OD9;
	}
	
	if (isHeaterOn == true)	// Heater output - PF8
	{
		GPIOF->ODR &= ~(GPIO_ODR_OD8);	// Active low
	}
	else
	{
		GPIOF->ODR |= GPIO_ODR_OD8;
	}
	
	if (isCoolingOn == true)	// Cooler output - PB8
	{
		GPIOB->ODR &= ~(GPIO_ODR_OD8);	// Active low
	}
	else
	{
		GPIOB->ODR |= GPIO_ODR_OD8;
	}
}