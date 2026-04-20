// Sound.c
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// your name Zuhayr Khondker, Qasim Bawa
// your data 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC.h"
#include "../inc/Timer.h"


const uint16_t *soundPointer;
uint32_t soundCounter;

void SysTick_IntArm(uint32_t period, uint32_t priority){
  // write this 
  SysTick->CTRL = 0;
  SysTick->LOAD = period - 1;
  SysTick->VAL = 0;
  SCB->SHP[1] = (SCB->SHP[1]&(~0xC0000000)) | (priority<<30); // set priroity
  SysTick->CTRL = 0x07; //turn on, enabvle interrupt, use system clock
}
// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5-bit DAC
void Sound_Init(void){
// write this
DAC_Init();
soundPointer = 0;
soundCounter = 0;
SysTick_IntArm(80000000/11025, 2); // 80MHz / 11025= 7256 --> count down 7256 clock ticks between each fire, so there are 11025 fires / second
SysTick->CTRL &= ~0x01; // disabled for now  
}
void SysTick_Handler(void){
  if(soundCounter > 0){
    DAC_Out(*soundPointer);
    soundPointer++;
    soundCounter--;
  } else{
    SysTick->CTRL &= ~0x01;
  }
}

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint16_t *pt, uint32_t count){
// write this
  soundPointer = pt;
  soundCounter = count;
  SysTick->CTRL |= 0x01; // handler firesl, play samples until we out
}


void Sound_Collect(void){
  Sound_Start(sound_collect, 3916);
}


void Sound_Splash(void){
  Sound_Start(sound_splash, 6221);
}

void Sound_Complete(void){
  Sound_Start(sound_complete, 9800);
}

void Sound_Chop(void){
  Sound_Start(sound_chop, 5449);
}

void Sound_GameOver(void){
  Sound_Start(sound_gameover, 8626);
}