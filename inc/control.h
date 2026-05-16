#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "button_handler.h"

void fanLightLogic(struct Button *fanButton, struct Button *lightButton, bool *fanControl, uint32_t *fan_lockout, uint8_t lightSensor);
void tempControl(bool autoControl, bool fanControl, float currentTemp, bool *isHeaterOn, bool *isCoolingOn, struct Button *fanButton);
void ledControl(struct Button fanButton, struct Button lightButton, bool isHeaterOn, bool isCoolingOn);

#endif // CONTROL_H
