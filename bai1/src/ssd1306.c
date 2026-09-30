#include "ssd1306.h"
#include "i2c.h"

static uint8_t framebuffer[
    SSD1306_WIDTH * SSD1306_HEIGHT / 8
];


/*
 * Send one command to SSD1306
 */
static void SSD1306_WriteCommand(uint8_t command)
{
    uint8_t data[2];

    data[0] = 0x00;
    data[1] = command;

    I2C1_Write(
        SSD1306_ADDRESS,
        data,
        2
    );
}


/*
 * Send data to SSD1306
 *
 * Control byte 0x40 means:
 * following bytes are display RAM data.
 */
static void SSD1306_WriteData(uint8_t *data,
                              uint16_t length)
{
    uint8_t buffer[17];

    while (length > 0)
    {
        uint8_t chunk;
        uint8_t i;

        chunk = (length > 16) ? 16 : length;

        buffer[0] = 0x40;

        for (i = 0; i < chunk; i++)
        {
            buffer[i + 1] = data[i];
        }

        I2C1_Write(
            SSD1306_ADDRESS,
            buffer,
            chunk + 1
        );

        data += chunk;
        length -= chunk;
    }
}


/*
 * SSD1306 initialization
 *
 * 128x64 OLED
 */
void SSD1306_Init(void)
{
    SSD1306_WriteCommand(0xAE); // Display OFF

    SSD1306_WriteCommand(0xD5);
    SSD1306_WriteCommand(0x80);

    SSD1306_WriteCommand(0xA8);
    SSD1306_WriteCommand(0x3F);

    SSD1306_WriteCommand(0xD3);
    SSD1306_WriteCommand(0x00);

    SSD1306_WriteCommand(0x40);

    SSD1306_WriteCommand(0x8D);
    SSD1306_WriteCommand(0x14);

    SSD1306_WriteCommand(0x20);
    SSD1306_WriteCommand(0x00);

    SSD1306_WriteCommand(0xA1);

    SSD1306_WriteCommand(0xC8);

    SSD1306_WriteCommand(0xDA);
    SSD1306_WriteCommand(0x12);

    SSD1306_WriteCommand(0x81);
    SSD1306_WriteCommand(0xCF);

    SSD1306_WriteCommand(0xD9);
    SSD1306_WriteCommand(0xF1);

    SSD1306_WriteCommand(0xDB);
    SSD1306_WriteCommand(0x40);

    SSD1306_WriteCommand(0xA4);

    SSD1306_WriteCommand(0xA6);

    SSD1306_WriteCommand(0xAF); // Display ON

    SSD1306_Clear();
    SSD1306_UpdateScreen();
}


/*
 * Clear framebuffer
 */
void SSD1306_Clear(void)
{
    uint16_t i;

    for (i = 0;
         i < sizeof(framebuffer);
         i++)
    {
        framebuffer[i] = 0x00;
    }
}


/*
 * Set one pixel
 *
 * color = 1 -> ON
 * color = 0 -> OFF
 */
void SSD1306_SetPixel(uint8_t x,
                      uint8_t y,
                      uint8_t color)
{
    uint16_t index;

    if (x >= SSD1306_WIDTH ||
        y >= SSD1306_HEIGHT)
    {
        return;
    }

    index = x + (y / 8) * SSD1306_WIDTH;

    if (color)
    {
        framebuffer[index] |=
            (1 << (y % 8));
    }
    else
    {
        framebuffer[index] &=
            ~(1 << (y % 8));
    }
}


/*
 * Send framebuffer to OLED
 */
void SSD1306_UpdateScreen(void)
{
    uint8_t page;

    for (page = 0; page < 8; page++)
    {
        SSD1306_WriteCommand(
            0xB0 + page
        );

        SSD1306_WriteCommand(0x00);

        SSD1306_WriteCommand(0x10);

        SSD1306_WriteData(
            &framebuffer[
                page * 128
            ],
            128
        );
    }
}
