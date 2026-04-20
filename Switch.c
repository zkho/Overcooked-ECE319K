#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include "Switch.h"

#define PB12INDEX 28

void Switch_Init(void){
  IOMUX->SECCFG.PINCM[PB12INDEX] = 0x00040081;
  IOMUX->SECCFG.PINCM[PB16INDEX] = 0x00040081;

  // Make PB2, PB3 inputs
  GPIOB->DOE31_0 &= ~((1<<12) | (1<<16));
}

uint32_t Switch_In(void){
  uint32_t in = GPIOB->DIN31_0;
  return ((in >> 12) & 0x01) | (((in >> 16) & 0x01) << 1);
}