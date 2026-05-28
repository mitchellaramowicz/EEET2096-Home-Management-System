/****************************************************
*			STM32F439 UART Logic Handler  								*
*			Developed for the STM32												*
*			Author: Mitchell Aramowicz										*
*			Header File																		*
*     Updated: 28/05/2026 	  											*
*****************************************************/

#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include <stdint.h>
#include <stdbool.h>
#include "button_handler.h"

#define OUTGOING_PACKET_LENGTH_BYTES 12

//******************************************************************************//
// Function: construct_packet()
// Input: float currentTemp - current temperature, 
//        struct Button lightButton - light button status, 
//        bool isHeaterOn - heater status, 
//        bool isCoolingOn - cooler status, 
//        struct Button fanButton - fan button status
// Return: uint8_t* - pointer to the constructed packet buffer
// Description: Constructs a data packet for UART transmission containing 
//              temperature and device status.
// Author: Mitchell Aramowicz
// *****************************************************************************//
uint8_t* construct_packet(float currentTemp, struct Button lightButton, bool isHeaterOn, bool isCoolingOn, struct Button fanButton);

//******************************************************************************//
// Function: send_data()
// Input: float currentTemp - current temperature, 
//        struct Button lightButton - light button status, 
//        bool isHeaterOn - heater status, 
//        bool isCoolingOn - cooler status, 
//        struct Button fanButton - fan button status
// Return: None
// Description: Constructs and transmits a status packet over USART3.
// Author: Mitchell Aramowicz
// *****************************************************************************//
void send_data(float currentTemp, struct Button lightButton, bool isHeaterOn, bool isCoolingOn, struct Button fanButton);

//******************************************************************************//
// Function: getByte()
// Input: None
// Return: int8_t - the received byte, or -1 if no byte is available
// Description: Non-blocking check for a received byte on USART3.
// Author: Mitchell Aramowicz
// *****************************************************************************//
int8_t getByte(void);

//******************************************************************************//
// Function: getPacket()
// Input: float currentTemp - current temperature, 
//        struct Button *lightButton - pointer to light button struct, 
//        bool *isHeaterOn - pointer to heater status, 
//        bool *isCoolingOn - pointer to cooler status, 
//        struct Button *fanButton - pointer to fan button struct
// Return: int - 1 if a full packet was processed, -1 otherwise
// Description: Processes incoming UART bytes to reconstruct a control packet.
// Author: Mitchell Aramowicz
// *****************************************************************************//
int getPacket(float currentTemp, struct Button *lightButton, bool *isHeaterOn, bool *isCoolingOn, struct Button *fanButton);

#endif // UART_HANDLER_H
