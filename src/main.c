
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
	RCC_init();
	LED_GPIO_config();
	Input_GPIO_config();
	ADC_config();
	UART_config();

	
  while (1)
  {
	
  }
} 

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

float adc_to_temp(uint16_t value)
{
	//return (55 - ((float)value)*(85/4095));
	return (55 - ((float)value)*(0.02075702076));
}
