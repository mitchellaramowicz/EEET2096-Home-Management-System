/****************************************************
*			STM32F439 Button Logic Handler  							*
*			Developed for the STM32												*
*			Author: Justin Nguyen													*
*			Source File																		*
*     Updated: 28/05/2026 	  											*
*****************************************************/

#include "button_handler.h"
#include <stdint.h>

//******************************************************************************//
// Function: button_process()
// Input: struct Button *button - pointer to button struct, 
//        uint8_t input - current GPIO input state
// Return: None
// Description: Processes a button press state machine, implementing debouncing 
//              and a 2-second lockout if the button is rapidly pressed.
// Author: Justin Nguyen
// *****************************************************************************//
void button_process(struct Button *button, uint8_t input)
{
	switch (button->state)
	{
		case NO_INPUT:
			// Detect falling edge - active low
			if (button->prevInput == 1 && input == 0)
			{
				// If button pressed reset timer and switch to pressed mode
				button->hold_time = 0;
				button->state = PRESSED;
			}

			break;

		case PRESSED:
			// Detect rising edge
			if (input == 1)
			{
				if (button->hold_time >= 10)
				{
					// If button has been released and 10 ms have passed change state
					button->state = CONFIRM;
				}
				else
				{
					// If button is released and not enough time then disregard result
					button->state = NO_INPUT;
				}
			}
			break;

		case CONFIRM:
			// Reset lockout timer
			button->lockout_time = 0;
			button->state = LOCKOUT;

			// Flip button output
			if (button->output == 0)
			{
				button->output = 1;
			}
			else if (button->output == 1)
			{
				button->output = 0;
			}
			break;

		case LOCKOUT:
			// No input for 2 seconds
			if (button->lockout_time >=2000)
			{
				button->state = NO_INPUT;
			}
			break;
	}
	button->prevInput = input;
}
