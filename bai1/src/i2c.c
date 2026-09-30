#include "i2c.h"

#define I2C_TIMEOUT 1000000UL

void I2C1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    /* Enable clocks */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_AFIO,
        ENABLE
    );

    RCC_APB1PeriphClockCmd(
        RCC_APB1Periph_I2C1,
        ENABLE
    );

    /*
     * PB6 -> I2C1_SCL
     * PB7 -> I2C1_SDA
     */
    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_6 | GPIO_Pin_7;

    GPIO_InitStructure.GPIO_Speed =
        GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Mode =
        GPIO_Mode_AF_OD;

    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* I2C configuration */
    I2C_InitStructure.I2C_Mode =
        I2C_Mode_I2C;

    I2C_InitStructure.I2C_DutyCycle =
        I2C_DutyCycle_2;

    I2C_InitStructure.I2C_OwnAddress1 =
        0x00;

    I2C_InitStructure.I2C_Ack =
        I2C_Ack_Enable;

    I2C_InitStructure.I2C_AcknowledgedAddress =
        I2C_AcknowledgedAddress_7bit;

    I2C_InitStructure.I2C_ClockSpeed =
        100000;

    I2C_Init(I2C1, &I2C_InitStructure);

    I2C_Cmd(I2C1, ENABLE);
}


uint8_t I2C1_Write(uint8_t device_address,
                   uint8_t *data,
                   uint16_t length)
{
    uint32_t timeout;

    if (length == 0)
        return 0;

    /* Wait until bus is free */
    timeout = I2C_TIMEOUT;

    while (I2C_GetFlagStatus(
               I2C1,
               I2C_FLAG_BUSY) != RESET)
    {
        if (--timeout == 0)
            return 0;
    }

    /* START */
    I2C_GenerateSTART(
        I2C1,
        ENABLE
    );

    timeout = I2C_TIMEOUT;

    while (I2C_CheckEvent(
               I2C1,
               I2C_EVENT_MASTER_MODE_SELECT)
           != SUCCESS)
    {
        if (--timeout == 0)
            return 0;
    }

    /* Send device address */
    I2C_Send7bitAddress(
        I2C1,
        device_address << 1,
        I2C_Direction_Transmitter
    );

    timeout = I2C_TIMEOUT;

    while (I2C_CheckEvent(
               I2C1,
               I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)
           != SUCCESS)
    {
        if (--timeout == 0)
            return 0;
    }

    /* Send data */
    while (length--)
    {
        I2C_SendData(I2C1, *data++);

        timeout = I2C_TIMEOUT;

        while (I2C_CheckEvent(
                   I2C1,
                   I2C_EVENT_MASTER_BYTE_TRANSMITTED)
               != SUCCESS)
        {
            if (--timeout == 0)
            {
                I2C_GenerateSTOP(I2C1, ENABLE);
                return 0;
            }
        }
    }

    /* STOP */
    I2C_GenerateSTOP(
        I2C1,
        ENABLE
    );

    return 1;
}
