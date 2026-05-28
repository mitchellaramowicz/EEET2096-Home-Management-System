/****************************************************
*			STM32F439 Button Logic Handler  							*
*			Developed for the STM32												*
*			Author: Justin Nguyen													*
*			Header File																		*
*     Updated: 28/05/2026 	  											*
*****************************************************/

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <stdint.h>

// Enum with button states for processing
enum ButtonState
{
	NO_INPUT,
	PRESSED,
	CONFIRM,
	LOCKOUT
};

// Struct that holds button information
struct Button 
{
	enum ButtonState state;
	uint32_t hold_time;			// Time that button has been held down
	uint32_t lockout_time;	// Time since lockout begun
	uint8_t prevInput;			// Previous reported input
	uint8_t output;					// Current toggled state - 0 for off, 1 for on
};

//******************************************************************************//
// Function: button_process()
// Input: struct Button *button - pointer to button structure, 
//        uint8_t input - current GPIO input state
// Return: None
// Description: Processes a button press state machine, implementing debouncing 
//              and a 2-second lockout if the button is rapidly pressed.
// Author: Justin Nguyen
// *****************************************************************************//
void button_process(struct Button *button, uint8_t input);

#endif // BUTTON_HANDLER_H
