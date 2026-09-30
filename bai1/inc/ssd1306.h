#ifndef __SSD1306_H
#define __SSD1306_H

#include "stm32f10x.h"

#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT   64
#define SSD1306_ADDRESS 0x3C

void SSD1306_Init(void);

void SSD1306_Clear(void);

void SSD1306_UpdateScreen(void);

void SSD1306_SetPixel(uint8_t x,
                      uint8_t y,
                      uint8_t color);

#endif
