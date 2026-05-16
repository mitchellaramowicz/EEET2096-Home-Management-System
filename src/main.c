
/********************************************
*			STM32F439 Main (C Startup File)  			*
*			Developed for the STM32								*
*			Author: 															*
*			Source File														*
*     Updated: 04/03/2026 	  							*
********************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "stm32f439xx.h"
#include "main.h"
#include "adc_handler.h"
#include "button_handler.h"
#include "uart_handler.h"
#include "control.h"

static struct Button fanButton;										//global button structs
static struct Button lightButton;

static bool isHeaterOn = false;
static bool isCoolingOn = false;
static volatile float currentTemp = 0;
static volatile uint32_t ms_counter = 0;

static bool autoControl = true;										// True - heater/cooler/fan controlled by temp, False - controlled by UART
static bool fanControl = false;										// True - Fan in manual off control, False - Fan in auto control
static uint32_t auto_lockout = 0;									// Timer for counting lockout time till auto override - max 10s
static uint32_t fan_lockout = 0;									// Timer for counting fan lockout time till auto override - max 10s

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
	lightButton.output = 0;									// Set light to be off by default

  while (1)
  {
		// Read ADC
		currentTemp = adc_to_temp(sample_ADC()); // UNCOMMENT FOR PRODUCTION
		// currentTemp = 20; // UNCOMMENT FOR SIMULATOR TESTING

		// Poll Switches
		volatile uint8_t fanInput = (GPIOB->IDR >> GPIO_IDR_ID0_Pos) & 0x01;					//Fan switch - PB0
		volatile uint8_t lightInput = (GPIOA->IDR >> GPIO_IDR_ID10_Pos) & 0x01 ;		  //Light switch - PA10
		
		if (autoControl == true)
		{
			// Allow fan button and light button to work when in auto control
			button_process(&fanButton, fanInput);
			button_process(&lightButton, lightInput);
		}
		else if (autoControl == false)
		{
			// If not in auto control only let light button work if 1 sec passed else UART takes precedence
			if (auto_lockout >= 1000)
			{
				button_process(&lightButton, lightInput);
			}
		}

  	// Send data to PC at 0.25 Hz
		if (ms_counter >= 4000) // If 4000ms has elapsed, send data to PC
		{
			ms_counter = 0;
			send_data(currentTemp, lightButton, isHeaterOn, isCoolingOn, fanButton);
		}

		// Do stuff based on light and fan switch state
		volatile uint8_t lightIntensity = (GPIOA->IDR >> GPIO_IDR_ID8_Pos) & 0x01;			// Light intensity sensor - PA8
																																										// Maybe add hold functionality later
		fanLightLogic(&fanButton, &lightButton, &fanControl, &fan_lockout, lightIntensity);

		// Check UART
		if(getPacket(currentTemp, &lightButton, &isHeaterOn, &isCoolingOn, &fanButton) == 1)
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
		tempControl(autoControl, fanControl, currentTemp, &isHeaterOn, &isCoolingOn, &fanButton);

		// Change LEDs based on settings
		ledControl(fanButton, lightButton, isHeaterOn, isCoolingOn);

		// Loop


  }
}

// TIM6 1 ms interrupt handler
void TIM6_DAC_IRQHandler(void)
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
