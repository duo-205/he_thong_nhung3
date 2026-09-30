#ifndef __MAX7219_H
#define __MAX7219_H

#include <stdint.h>

void MAX7219_Init(void);
void MAX7219_Write(uint8_t address, uint8_t data);
void MAX7219_Clear(void);
void MAX7219_SetIntensity(uint8_t intensity);
void MAX7219_DisplayNumber(uint8_t number);

#endif
