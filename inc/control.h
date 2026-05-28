/****************************************************
*			STM32F439 Application Logic  									*
*			Developed for the STM32												*
*			Author: Justin Nguyen													*
*			Header File																		*
*     Updated: 28/05/2026 	  											*
*****************************************************/

#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "button_handler.h"

//******************************************************************************//
// Function: fanLightLogic()
// Input: struct Button *fanButton - pointer to fan button struct, 
//        struct Button *lightButton - pointer to light button struct, 
//        bool *fanControl - pointer to fan manual control status, 
//        uint32_t *fan_lockout - pointer to fan lockout timer, 
//        uint8_t lightSensor - light intensity sensor input
// Return: None
// Description: Handles the fan and light logic.
// Author: Justin Nguyen
// *****************************************************************************//
void fanLightLogic(struct Button *fanButton, struct Button *lightButton, bool *fanControl, uint32_t *fan_lockout, uint8_t lightSensor);

//******************************************************************************//
// Function: tempControl()
// Input: bool autoControl - automatic control status, 
//        bool fanControl - manual fan override status, 
//        float currentTemp - current temperature, 
//        bool *isHeaterOn - pointer to heater status, 
//        bool *isCoolingOn - pointer to cooler status, 
//        struct Button *fanButton - pointer to fan button struct
// Return: None
// Description: Controls the heater, cooler, and fan based on the current temperature.
// Author: Justin Nguyen
// *****************************************************************************//
void tempControl(bool autoControl, bool fanControl, float currentTemp, bool *isHeaterOn, bool *isCoolingOn, struct Button *fanButton);

//******************************************************************************//
// Function: ledControl()
// Input: struct Button fanButton - fan button struct, 
//        struct Button lightButton - light button struct, 
//        bool isHeaterOn - heater status, 
//        bool isCoolingOn - cooler status
// Return: None
// Description: Updates the hardware LEDs based on the current application status.
// Author: Justin Nguyen
// *****************************************************************************//
void ledControl(struct Button fanButton, struct Button lightButton, bool isHeaterOn, bool isCoolingOn);

#endif // CONTROL_H
