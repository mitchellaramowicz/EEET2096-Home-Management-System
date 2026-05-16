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
	uint32_t hold_time;				// Time that button has been held down
	uint32_t lockout_time;		// Time since lockout begun
	uint8_t prevInput;				// Previous reported input
	uint8_t output;						// Current toggled state - 0 for off, 1 for on
};

void button_process(struct Button *button, uint8_t input);

#endif // BUTTON_HANDLER_H
