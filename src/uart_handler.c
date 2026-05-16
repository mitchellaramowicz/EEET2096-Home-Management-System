#include "uart_handler.h"
#include "stm32f439xx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

uint8_t* construct_packet(float currentTemp, struct Button lightButton, bool isHeaterOn, bool isCoolingOn, struct Button fanButton)
{
	static uint8_t packet[OUTGOING_PACKET_LENGTH_BYTES];
	uint8_t index = 0;
	packet[index++] = '&';
	packet[index++] = '~';

	// Choose the sign for the temperature
	char sign = '+';
	if (currentTemp < 0)
	{
		sign = '-';
	}

	// Get the absolute value of the temperature
	float absTemp = currentTemp;
	if (currentTemp < 0)
	{
		absTemp = -currentTemp;
	}

	char tempBuffer[8]; // Set up temporary buffer for temperature string
	snprintf(tempBuffer, sizeof(tempBuffer), "%c%05.2f", sign, absTemp);

	// Copy temperature string into packet
	for (uint8_t i = 0; i < 6; i++) {
		packet[index++] = (uint8_t)tempBuffer[i];
	}

	packet[index++] = '~';
	packet[index++] = '0';
	packet[index++] = '1';
	
	if (lightButton.output == 0) {
		packet[index++] = '0';
	} else {
		packet[index++] = '1';
	}
	
	if (isHeaterOn == 0) {
		packet[index++] = '0';
	} else {
		packet[index++] = '1';
	}
	
	if (isCoolingOn == 0) {
		packet[index++] = '0';
	} else {
		packet[index++] = '1';
	}
	
	if (fanButton.output == 0) {
		packet[index++] = '0';
	} else {
		packet[index++] = '1';
	}
	
	packet[index++] = '0';
	packet[index++] = '1';
	packet[index++] = 0x0D;
	packet[index] = 0x0A;
	
	return packet;
}

void send_data(float currentTemp, struct Button lightButton, bool isHeaterOn, bool isCoolingOn, struct Button fanButton)
{
	uint8_t* packet = construct_packet(currentTemp, lightButton, isHeaterOn, isCoolingOn, fanButton);

	for (uint8_t i = 0; i < OUTGOING_PACKET_LENGTH_BYTES; i++) {
		uint32_t timeout = 10000;	// idk if this is the right timeout value also make this a hardware timer
		while ((USART3->SR & USART_SR_TXE) == 0 && timeout > 0) {
			timeout--;
		}
		if (timeout > 0) {
			USART3->DR = packet[i];
		}
	}
}

int8_t getByte(void)
{
	int8_t receivedByte = -1;

	if (USART3->SR & USART_SR_RXNE)
	{
		receivedByte = USART3->DR;
	}

	return receivedByte;
}

int getPacket(float currentTemp, struct Button *lightButton, bool *isHeaterOn, bool *isCoolingOn, struct Button *fanButton)
{
	static uint8_t state = 0;
	static uint8_t a, b, c, d;
	int8_t receivedByte = getByte();
	uint8_t receivedDigit;

	if (receivedByte != -1)
	{
		// Error checking - discard packet if CR or LF is received unexpectedly
		if ((receivedByte == 0x0D || receivedByte == 0x0A) && state != 9)
		{
			state = 0;
			return -1;
		}

		switch (state)
		{
			case 0: // Waiting for '&'
				if (receivedByte == '&')
				{
					state = 1;
				}
				break;

			case 1: // Waiting for '0'
				if (receivedByte == '0')
				{
					state = 2;
				}
				else
				{
					state = 0;
				}
				break;

			case 2: // Waiting for '1'
				if (receivedByte == '1')
				{
					state = 3;
				}
				else
				{
					state = 0;
				}
				break;

			case 3: // Receiving 'a' (light output)
				receivedDigit = (uint8_t)receivedByte;
				if (receivedDigit == '0' || receivedDigit == '1')
				{
					a = receivedDigit - '0';
					state = 4;
				}
				else
				{
					state = 0;
				}
				break;

			case 4: // Receiving 'b' (heater output)
				receivedDigit = (uint8_t)receivedByte;
				if (receivedDigit == '0' || receivedDigit == '1')
				{
					b = receivedDigit - '0';
					state = 5;
				}
				else
				{
					state = 0;
				}
				break;

			case 5: // Receiving 'c' (cooling output)
				receivedDigit = (uint8_t)receivedByte;
				if (receivedDigit == '0' || receivedDigit == '1')
				{
					c = receivedDigit - '0';
					state = 6;
				}
				else
				{
					state = 0;
				}
				break;

			case 6: // Receiving 'd' (fan output)
				receivedDigit = (uint8_t)receivedByte;
				if (receivedDigit == '0' || receivedDigit == '1')
				{
					d = receivedDigit - '0';
					state = 7;
				}
				else
				{
					state = 0;
				}
				break;

			case 7: // Waiting for first '0'
				if (receivedByte == '0')
				{
					state = 8;
				}
				else
				{
					state = 0;
				}
				break;

			case 8: // Waiting for second '0'
				if (receivedByte == '0')
				{
					state = 9;
				}
				else
				{
					state = 0;
				}
				break;

			case 9: // Waiting for CR (0x0D) or LF (0x0A)
				if (receivedByte == 0x0D || receivedByte == 0x0A)
				{
					lightButton->output = a;
					
					if (currentTemp >= 15 && currentTemp <= 30) // WE ARE ASSUMING IT IS INCLUSIVE OF 15 AND 30 DEGREES
					{
						*isHeaterOn = b;
						*isCoolingOn = c;
						fanButton->output = d;
					}
					state = 0;
					return 1; // Successfully received packet
				}
				state = 0;
				break;

			default:
				state = 0;
				break;
		}
	}

	return -1; // No valid packet received yet
}
