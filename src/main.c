
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


  while (1)
  {
		// Read ADC
		currentTemp = adc_to_temp(sample_ADC());

		// Poll Switches
		uint8_t fanInput = GPIOB->IDR & GPIO_IDR_ID0;				//Fan switch - PB0
		uint8_t lightInput = GPIOA->IDR & GPIO_IDR_ID10;		//Light switch - PA10
		debounce(&fanButton, fanInput);
		debounce(&lightButton, lightInput);

  		// Send data to PC at 0.25 Hz
  		if (TIM6->SR & TIM_UIF_UIF) // If timer has expired
  		{
  			TIM6->SR &= ~TIM_SR_UIF;   // Clear UIF flag
  			ms_counter++;

  			if (ms_counter >= 4000) // If 4000ms has elapsed, send data to PC
  			{
  				ms_counter = 0;
  				send_data();
  			}
  		}

		// Do stuff based on light and fan switch state

		/*
		//Can check button states like the following:
		if (fanButton.output == 1)
		{
			// do stuff
		}
		*/


		// Check UART
		getPacket();

		// Do stuff based on UART input

		// Temp control - control fan + heater setting based on temp

		// Change LEDs based on settings

		// Loop


  }
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
	}

	return -1;
}