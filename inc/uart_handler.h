#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include <stdint.h>
#include <stdbool.h>
#include "button_handler.h"

#define OUTGOING_PACKET_LENGTH_BYTES 12

uint8_t* construct_packet(float currentTemp, struct Button lightButton, bool isHeaterOn, bool isCoolingOn, struct Button fanButton);
void send_data(float currentTemp, struct Button lightButton, bool isHeaterOn, bool isCoolingOn, struct Button fanButton);
int8_t getByte(void);
int getPacket(float currentTemp, struct Button *lightButton, bool *isHeaterOn, bool *isCoolingOn, struct Button *fanButton);

#endif // UART_HANDLER_H
