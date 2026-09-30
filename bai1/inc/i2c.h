#ifndef __I2C_H
#define __I2C_H

#include "stm32f10x.h"

void I2C1_Config(void);

uint8_t I2C1_Write(uint8_t device_address,
                   uint8_t *data,
                   uint16_t length);

#endif
