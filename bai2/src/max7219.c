#include "max7219.h"
#include "stm32f10x.h"

/*
 * MAX7219 connection:
 *
 * STM32F103       MAX7219
 * ------------------------
 * PA5 SPI1_SCK -> CLK
 * PA7 SPI1_MOSI -> DIN
 * PA4          -> CS/LOAD
 */

/* Chip Select pin */
#define MAX7219_CS_LOW()   GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define MAX7219_CS_HIGH()  GPIO_SetBits(GPIOA, GPIO_Pin_4)


static void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    /* Enable GPIOA and SPI1 clocks */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_SPI1,
        ENABLE
    );

    /*
     * PA5 = SPI1_SCK
     * PA7 = SPI1_MOSI
     */
    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_5 |
        GPIO_Pin_7;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*
     * PA4 = CS
     */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* CS initially HIGH */
    MAX7219_CS_HIGH();

    /*
     * SPI1 configuration
     */
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;

    /*
     * MAX7219:
     * CPOL = 0
     * CPHA = 0
     */
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;

    /*
     * Software NSS
     */
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;

    /*
     * SPI clock:
     * 72 MHz / 16 = 4.5 MHz
     */
    SPI_InitStructure.SPI_BaudRatePrescaler =
        SPI_BaudRatePrescaler_16;

    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}


/*
 * Send one byte through SPI1
 */
static void SPI1_SendByte(uint8_t data)
{
    /*
     * Wait until transmit buffer is empty
     */
    while (SPI_I2S_GetFlagStatus(
        SPI1,
        SPI_I2S_FLAG_TXE
    ) == RESET)
    {
    }

    SPI_I2S_SendData(SPI1, data);

    /*
     * Wait until transmission is complete
     */
    while (SPI_I2S_GetFlagStatus(
        SPI1,
        SPI_I2S_FLAG_BSY
    ) == SET)
    {
    }
}


/*
 * Write one register of MAX7219
 *
 * address = register address
 * data    = register value
 */
void MAX7219_Write(uint8_t address, uint8_t data)
{
    MAX7219_CS_LOW();

    SPI1_SendByte(address);
    SPI1_SendByte(data);

    MAX7219_CS_HIGH();
}


/*
 * Initialize MAX7219
 */
void MAX7219_Init(void)
{
    SPI1_Init();

    /*
     * Shutdown register
     * 0 = shutdown
     * 1 = normal operation
     */
    MAX7219_Write(0x0C, 0x01);

    /*
     * Decode mode
     * 0 = no decode
     */
    MAX7219_Write(0x09, 0x00);

    /*
     * Scan limit
     * 0x07 = scan all 8 digits/rows
     */
    MAX7219_Write(0x0B, 0x07);

    /*
     * Intensity
     * 0x00 -> minimum
     * 0x0F -> maximum
     */
    MAX7219_Write(0x0A, 0x08);

    /*
     * Display test OFF
     */
    MAX7219_Write(0x0F, 0x00);

    /*
     * Clear display
     */
    MAX7219_Clear();
}


/*
 * Clear all 8 rows
 */
void MAX7219_Clear(void)
{
    uint8_t i;

    for (i = 1; i <= 8; i++)
    {
        MAX7219_Write(i, 0x00);
    }
}


/*
 * Set brightness
 *
 * intensity:
 * 0  -> minimum
 * 15 -> maximum
 */
void MAX7219_SetIntensity(uint8_t intensity)
{
    if (intensity > 15)
    {
        intensity = 15;
    }

    MAX7219_Write(0x0A, intensity);
}


/*
 * Display number 0-9
 *
 * Each number is represented by
 * an 8x8 bitmap.
 */
void MAX7219_DisplayNumber(uint8_t number)
{
    /*
     * 8 rows for each number
     */
    static const uint8_t numbers[10][8] =
    {
        {
            0x3C,
            0x66,
            0x6E,
            0x76,
            0x66,
            0x66,
            0x3C,
            0x00
        },

        {
            0x18,
            0x38,
            0x18,
            0x18,
            0x18,
            0x18,
            0x7E,
            0x00
        },

        {
            0x3C,
            0x66,
            0x06,
            0x0C,
            0x18,
            0x30,
            0x7E,
            0x00
        },

        {
            0x3C,
            0x66,
            0x06,
            0x1C,
            0x06,
            0x66,
            0x3C,
            0x00
        },

        {
            0x0C,
            0x1C,
            0x3C,
            0x6C,
            0x7E,
            0x0C,
            0x0C,
            0x00
        },

        {
            0x7E,
            0x60,
            0x7C,
            0x06,
            0x06,
            0x66,
            0x3C,
            0x00
        },

        {
            0x1C,
            0x30,
            0x60,
            0x7C,
            0x66,
            0x66,
            0x3C,
            0x00
        },

        {
            0x7E,
            0x06,
            0x0C,
            0x18,
            0x30,
            0x30,
            0x30,
            0x00
        },

        {
            0x3C,
            0x66,
            0x66,
            0x3C,
            0x66,
            0x66,
            0x3C,
            0x00
        },

        {
            0x3C,
            0x66,
            0x66,
            0x3E,
            0x06,
            0x0C,
            0x38,
            0x00
        }
    };

    uint8_t row;

    if (number > 9)
    {
        return;
    }

    for (row = 0; row < 8; row++)
    {
        MAX7219_Write(
            row + 1,
            numbers[number][row]
        );
    }
}

