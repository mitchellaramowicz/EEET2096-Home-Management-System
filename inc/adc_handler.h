#ifndef ADC_HANDLER_H
#define ADC_HANDLER_H

#include <stdint.h>

uint16_t sample_ADC(void);
float adc_to_temp(uint16_t value);

#endif // ADC_HANDLER_H
