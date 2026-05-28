/****************************************************
*			STM32F439 ADC Logic Handler  									*
*			Developed for the STM32												*
*			Author: Justin Nguyen													*
*			Header File																		*
*     Updated: 28/05/2026 	  											*
*****************************************************/

#ifndef ADC_HANDLER_H
#define ADC_HANDLER_H

#include <stdint.h>

//******************************************************************************//
// Function: sample_ADC()
// Input: None
// Return: uint16_t - ADC sample result
// Description: Triggers an ADC conversion and waits for completion to return 
//              the 12-bit digital value.
// Author: Justin Nguyen
// *****************************************************************************//
uint16_t sample_ADC(void);

//******************************************************************************//
// Function: adc_to_temp()
// Input: uint16_t - raw ADC value
// Return: float - The current temperature in degrees Celsius
// Description: Converts the 12-bit ADC value to a temperature value using 
//              a linear calibration.
// Author: Justin Nguyen
// *****************************************************************************//
float adc_to_temp(uint16_t value);

#endif // ADC_HANDLER_H
