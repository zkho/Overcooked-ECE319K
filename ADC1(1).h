#ifndef __ADC1_H__
#define __ADC1_H__

#include <stdint.h>

void ADCinit(void);
uint32_t ADCin(void);
void ADCin2(uint32_t *x, uint32_t *y);
uint32_t ADCinX(void);
uint32_t ADCinY(void);
uint32_t Convert(uint32_t input);
float FloatConvert(uint32_t input);

#endif