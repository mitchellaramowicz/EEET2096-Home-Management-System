#include "adc_handler.h"
#include "stm32f439xx.h"
#include <stdint.h>

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
	return (55.0f - ((float)value)*(0.02075702076f));
}
