#include <ti/devices/msp/msp.h>
#include <stdint.h>
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "ADC1.h"

#define ADCVREF_VDDA 0x00000000

#define JOYX_CH 5   
#define JOYY_CH 4   
#define PB17INDEX 42
#define PB18INDEX 45

static uint32_t ADCinChannel(uint32_t channel){
  ADC1->ULLMEM.MEMCTL[0] = ADCVREF_VDDA + channel;

  ADC1->ULLMEM.CTL0 |= 0x00000001;
  ADC1->ULLMEM.CTL1 |= 0x00000100;

  volatile uint32_t delay = ADC1->ULLMEM.STATUS;
  (void)delay;

  while((ADC1->ULLMEM.STATUS & 0x01) == 0x01){
  }

  return ADC1->ULLMEM.MEMRES[0] & 0x0FFF;
}

void ADCinit(void){
  IOMUX->SECCFG.PINCM[PB17INDEX] = 0x00040081;
  IOMUX->SECCFG.PINCM[PB18INDEX] = 0x00040081;

  GPIOB->DOE31_0 &= ~((1<<16) | (1<<18));

  ADC1->ULLMEM.GPRCM.RSTCTL = 0xB1000003;
  ADC1->ULLMEM.GPRCM.PWREN  = 0x26000001;
  Clock_Delay(24);

  ADC1->ULLMEM.GPRCM.CLKCFG = 0xA9000000;
  ADC1->ULLMEM.CLKFREQ      = 7;
  ADC1->ULLMEM.CTL0         = 0x03010000;
  ADC1->ULLMEM.CTL1         = 0x00000000;
  ADC1->ULLMEM.CTL2         = 0x00000000;

  ADC1->ULLMEM.MEMCTL[0]    = ADCVREF_VDDA + JOYX_CH;

  ADC1->ULLMEM.SCOMP0 = 0;
  ADC1->ULLMEM.GEN_EVENT.IMASK = 0;
}

uint32_t ADCin(void){
  return ADCinChannel(JOYX_CH);
}

void ADCin2(uint32_t *x, uint32_t *y){
  *x = ADCinChannel(JOYX_CH);
  *y = ADCinChannel(JOYY_CH);
}

uint32_t ADCinX(void){
  return ADCinChannel(JOYX_CH);
}

uint32_t ADCinY(void){
  return ADCinChannel(JOYY_CH);
}

uint32_t Convert(uint32_t input){
  int32_t position;
  position = ((1802*(int32_t)input) >> 12) + 33;
  if(position < 0) position = 0;
  if(position > 2000) position = 2000;
  return (uint32_t)position;
}

float FloatConvert(uint32_t input){
  return 0.00048828125f*input - 0.0001812345f;
}