/********************************************
*			STM32F439 Main (C Header File)  			*
*			Developed for the STM32								*
*			Author: Dr. Glenn Matthews						*
*			Header File														*
********************************************/

#ifndef MAIN_H
#define MAIN_H

// Compiler pragmas
#include "boardSupport.h"

// Setup functions
void RCC_init(void);
void LED_GPIO_config(void);
void Input_GPIO_config(void);
void ADC_config(void);
void UART_config(void);
void timer6_config(void);

#endif // MAIN_H
