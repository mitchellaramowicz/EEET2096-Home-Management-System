
/********************************************
*			STM32F439 Main (C Startup File)  			*
*			Developed for the STM32								*
*			Author: 						*
*			Source File														*
*     Updated: 04/03/2026 	  							*
********************************************/

#include <stdint.h>
#include "main.h"

uint16_t sample_ADC();
float adc_to_temp(uint16_t value);
void debounce(struct Button *button, uint8_t input);

struct Button fanButton;			//global button structs 
struct Button lightButton;



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
		float temp_value = adc_to_temp(sample_ADC());
		
		// Poll Switches
		uint8_t fanInput = GPIOB->IDR & GPIO_IDR_ID0;				//Fan switch - PB0
		uint8_t lightInput = GPIOA->IDR & GPIO_IDR_ID10;		//Light switch - PA10
		debounce(&fanButton, fanInput);
		debounce(&lightButton, lightInput);
		
		// Do stuff based on light and fan switch state
		
		/*
		//Can check button states like the following:
		if (fanButton.output == 1)
		{
			// do stuff
		}
		*/
		
		
		// Check UART
		
		// Do stuff based on UART input
		
		// Temp control - control fan + heater setting based on temp
		
		// Change LEDs based on settings
		
		// If 0.25 Hz passed -> output UART packet to PC
		
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
