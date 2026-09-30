#include "stm32f10x.h"
#include "max7219.h"


static void Delay_ms(uint32_t ms)
{
    volatile uint32_t i;

    while (ms--)
    {
        /*
         * Approximate delay for STM32F103
         * running at 72 MHz.
         */
        for (i = 0; i < 7200; i++)
        {
            __NOP();
        }
    }
}


int main(void)
{
    /*
     * Initialize MAX7219
     */
    MAX7219_Init();

    /*
     * Set brightness
     */
    MAX7219_SetIntensity(8);

    while (1)
    {
        /*
         * Display number 5
         */
        MAX7219_DisplayNumber(5);

        /*
         * Keep displaying
         */
        Delay_ms(1000);
    }
}

