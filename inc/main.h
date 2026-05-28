/********************************************
*			STM32F439 Main (C Header File)  			*
*			Developed for the STM32								*
*			Author: Dr. Glenn Matthews						*
*			Edited By: Mitchell Aramowicz					*
*			Header File														*
********************************************/

#ifndef MAIN_H
#define MAIN_H

// Compiler pragmas
#include "boardSupport.h"

// Setup functions
void RCC_Config(void);
void LED_GPIO_Config(void);
void Input_GPIO_Config(void);
void ADC_Config(void);
void UART_Config(void);
void TIM_Config(void);

extern volatile uint32_t ms_counter;

#endif // MAIN_H
