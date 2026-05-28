/****************************************************
*			STM32F439 Application Logic  									*
*			Developed for the STM32												*
*			Author: Justin Nguyen													*
*			Source File																		*
*     Updated: 28/05/2026 	  											*
*****************************************************/

#include "control.h"
#include "stm32f439xx.h"
#include <stdbool.h>

//******************************************************************************//
// Function: fanLightLogic()
// Input: struct Button *fanButton - pointer to fan button struct, 
//        struct Button *lightButton - pointer to light button struct, 
//        bool *fanControl - pointer to fan manual control status, 
//        uint32_t *fan_lockout - pointer to fan lockout timer, 
//        uint8_t lightSensor - light intensity sensor input
// Return: None
// Description: Handles the fan and light logic.
//							- If the fan is in auto control and the fan is switched off, then switch to manual control for 10 seconds.
//							- Otherwise, if the fan is in manual control and 10 seconds has passed, then turn on auto control and turn fan back on.
//							- Otherwise, turn auto control off.
//							- Then, if light is detected and the light button is pressed, keep the light off.
// Author: Justin Nguyen
// *****************************************************************************//
void fanLightLogic(struct Button *fanButton, struct Button *lightButton, bool *fanControl, uint32_t *fan_lockout, uint8_t lightSensor)
{
	// Fan control
	if (fanButton->output == 0)	// Fan is switched off
	{
		if (*fanControl == false)
		{
			// If fan has been switched off for the first time enable 10s countdown and manual control
			*fan_lockout = 0;
			*fanControl = true;
		}
		else if (*fanControl == true)	// Fan is in manual control
		{
			if (*fan_lockout >= 10000)
			{
				// If 10 seconds passed, turn fan back on and turn off manual control
				*fanControl = false;
				fanButton->output = 1;
			}
		}
	}
	else
	{
		*fanControl = false;
	}
	
	// Light control
	if ((lightButton->output == 1) && (lightSensor == 0))
	{
		// If light button is pressed and light sensor detects light (active low) then switch off light button
		lightButton->output = 0;
	}
}

//******************************************************************************//
// Function: tempControl()
// Input : bool autoControl - automatic control status, 
//         bool fanControl - manual fan override status, 
//         float currentTemp - current temperature, 
//         bool *isHeaterOn - pointer to heater status, 
//         bool *isCoolingOn - pointer to cooler status, 
//         struct Button *fanButton - pointer to fan button struct
// Return : None
// Description : Controls the heater, cooler, and fan based on the current temperature
//               when in automatic control mode.
// Author: Justin Nguyen
// *****************************************************************************//
void tempControl(bool autoControl, bool fanControl, float currentTemp, bool *isHeaterOn, bool *isCoolingOn, struct Button *fanButton)
{
	if (autoControl == true)
	{
		if (fanControl == false)			// If fan off manual overridde is not on
		{
			if (currentTemp < 22)
			{
				// If temp less then 22, heater + fan ON
				*isHeaterOn = true;
				*isCoolingOn = false;
				fanButton->output = 1;
			}
			else if (currentTemp > 24)
			{
				// If temp more then 24, cooler + fan ON
				*isHeaterOn = false;
				*isCoolingOn = true;
				fanButton->output = 1;
			}
			else
			{
				// Else in hysteresis band, heater + cooler OFF, fan ON
				*isHeaterOn = false;
				*isCoolingOn = false;
				fanButton->output = 1;
			}
		}
		else if (fanControl == true)	// If fan off manual overridde is on
		{
			if (currentTemp < 22)
			{
				// If temp less then 22, heater ON + fan OFF
				*isHeaterOn = true;
				*isCoolingOn = false;
				fanButton->output = 0;
			}
			else if (currentTemp > 24)
			{
				// If temp more then 24, cooler ON + fan OFF
				*isHeaterOn = false;
				*isCoolingOn = true;
				fanButton->output = 0;
			}
			else
			{
				// Else in hysteresis band, heater + cooler OFF, fan OFF
				*isHeaterOn = false;
				*isCoolingOn = false;
				fanButton->output = 0;
			}
		}
	}
}

//******************************************************************************//
// Function: ledControl()
// Input : struct Button fanButton - fan button struct, 
//         struct Button lightButton - light button struct, 
//         bool isHeaterOn - heater status, 
//         bool isCoolingOn - cooler status
// Return : None
// Description : Updates the hardware LEDs based on the current application status.
// Author: Justin Nguyen
// *****************************************************************************//
void ledControl(struct Button fanButton, struct Button lightButton, bool isHeaterOn, bool isCoolingOn)
{
	if (fanButton.output == 1)				// Fan control output - PB1
	{
		GPIOB->ODR &= ~(GPIO_ODR_OD1);	// Active low
	}
	else
	{
		GPIOB->ODR |= GPIO_ODR_OD1;
	}
	
	if (lightButton.output == 1)			// Light control output - PA9
	{
		GPIOA->ODR &= ~(GPIO_ODR_OD9);	// Active low
	}
	else
	{
		GPIOA->ODR |= GPIO_ODR_OD9;
	}
	
	if (isHeaterOn == true)						// Heater output - PF8
	{
		GPIOF->ODR &= ~(GPIO_ODR_OD8);	// Active low
	}
	else
	{
		GPIOF->ODR |= GPIO_ODR_OD8;
	}
	
	if (isCoolingOn == true)					// Cooler output - PB8
	{
		GPIOB->ODR &= ~(GPIO_ODR_OD8);	// Active low
	}
	else
	{
		GPIOB->ODR |= GPIO_ODR_OD8;
	}
}
