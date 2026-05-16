#include "control.h"
#include "stm32f439xx.h"
#include <stdbool.h>

void fanLightLogic(struct Button *fanButton, struct Button *lightButton, bool *fanControl, uint32_t *fan_lockout, uint8_t lightSensor)
{
	// Fan control
	if (fanButton->output == 0)					// Fan is switched off
	{
		if (*fanControl == false)
		{
			// If fan has been switched off for the first time enable 10s countdown and manual control
			*fan_lockout = 0;
			*fanControl = true;
		}
		else if (*fanControl == true)			// Fan is in manual control
		{
			if (*fan_lockout >= 10000)
			{
				// If 10s passed turn fan back on and turn off manual control
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
