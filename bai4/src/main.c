#include "stm32f10x.h"
#include <stdint.h>
#include <stdio.h>

/* ============================================================
 * Configuration
 * ============================================================ */

#define ADC_BUFFER_SIZE     100

#define UART_BAUDRATE       115200

/*
 * STM32F103C8T6:
 * ADC1_IN0 = PA0
 *
 * TIM3:
 * 72 MHz / (7199 + 1) = 10 kHz
 * 10 kHz / (99 + 1) = 100 Hz
 *
 * => ADC sampling frequency = 100 Hz
 */

/* ADC DMA buffer */
volatile uint16_t adc_buffer[ADC_BUFFER_SIZE];

/* Flags */
volatile uint8_t adc_half_ready = 0;
volatile uint8_t adc_full_ready = 0;


/* ============================================================
 * UART1
 * PA9  = TX
 * PA10 = RX
 * ============================================================ */

static void UART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* Enable GPIOA and USART1 clocks */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_USART1,
        ENABLE
    );

    /* PA9 = USART1_TX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA10 = USART1_RX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART configuration */
    USART_InitStructure.USART_BaudRate = UART_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode =
        USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}


/* ============================================================
 * UART send
 * ============================================================ */

static void UART1_SendChar(char c)
{
    USART_SendData(USART1, (uint16_t)c);

    while (USART_GetFlagStatus(
        USART1,
        USART_FLAG_TXE
    ) == RESET)
    {
    }
}


static void UART1_SendString(const char *str)
{
    while (*str)
    {
        UART1_SendChar(*str++);
    }
}


/* ============================================================
 * Send one ADC value
 * Format:
 *
 * 0
 * 123
 * 2048
 * 4095
 *
 * Each value ends with \n\r
 * ============================================================ */

static void UART1_SendADCValue(uint16_t value)
{
    char buffer[16];
    int i = 0;

    /*
     * Convert number to ASCII manually.
     * This avoids using sprintf/printf and keeps
     * the program small.
     */

    if (value >= 1000)
    {
        buffer[i++] = (char)('0' + (value / 1000));
        value %= 1000;

        buffer[i++] = (char)('0' + (value / 100));
        value %= 100;

        buffer[i++] = (char)('0' + (value / 10));
        value %= 10;

        buffer[i++] = (char)('0' + value);
    }
    else if (value >= 100)
    {
        buffer[i++] = (char)('0' + (value / 100));
        value %= 100;

        buffer[i++] = (char)('0' + (value / 10));
        value %= 10;

        buffer[i++] = (char)('0' + value);
    }
    else if (value >= 10)
    {
        buffer[i++] = (char)('0' + (value / 10));
        value %= 10;

        buffer[i++] = (char)('0' + value);
    }
    else
    {
        buffer[i++] = (char)('0' + value);
    }

    buffer[i++] = '\n';
    buffer[i++] = '\r';
    buffer[i] = '\0';

    UART1_SendString(buffer);
}


/* ============================================================
 * Send a section of ADC buffer
 *
 * start = first sample
 * count = number of samples
 * ============================================================ */

static void UART1_SendADCBuffer(
    uint16_t start,
    uint16_t count
)
{
    uint16_t i;

    for (i = 0; i < count; i++)
    {
        UART1_SendADCValue(adc_buffer[start + i]);
    }
}


/* ============================================================
 * GPIO for ADC
 *
 * PA0 = ADC1_IN0
 * ============================================================ */

static void ADC_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA,
        ENABLE
    );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/* ============================================================
 * ADC1 configuration
 *
 * ADC1_IN0 = PA0
 *
 * External trigger:
 * TIM3 TRGO
 *
 * Conversion:
 * 1 channel
 *
 * DMA:
 * enabled
 * ============================================================ */

static void ADC1_Config(void)
{
    ADC_InitTypeDef ADC_InitStructure;

    /* Enable ADC1 clock */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_ADC1,
        ENABLE
    );

    /*
     * ADC clock:
     *
     * PCLK2 = 72 MHz
     * ADC clock = 72 / 6 = 12 MHz
     *
     * 12 MHz is within STM32F103 ADC clock limit.
     */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    /* ADC configuration */
    ADC_InitStructure.ADC_Mode =
        ADC_Mode_Independent;

    ADC_InitStructure.ADC_ScanConvMode =
        DISABLE;

    ADC_InitStructure.ADC_ContinuousConvMode =
        DISABLE;

    /*
     * ADC conversion is triggered by TIM3 TRGO.
     */
    ADC_InitStructure.ADC_ExternalTrigConv =
        ADC_ExternalTrigConv_T3_TRGO;

    ADC_InitStructure.ADC_DataAlign =
        ADC_DataAlign_Right;

    ADC_InitStructure.ADC_NbrOfChannel =
        1;

    ADC_Init(ADC1, &ADC_InitStructure);

    /*
     * Configure channel 0.
     *
     * Sample time = 55.5 cycles
     */
    ADC_RegularChannelConfig(
        ADC1,
        ADC_Channel_0,
        1,
        ADC_SampleTime_55Cycles5
    );

    /*
     * Enable ADC DMA request.
     */
    ADC_DMACmd(ADC1, ENABLE);

    /*
     * Enable external trigger.
     */
    ADC_ExternalTrigConvCmd(
        ADC1,
        ENABLE
    );

    /*
     * Enable ADC.
     */
    ADC_Cmd(ADC1, ENABLE);

    /*
     * Reset calibration.
     */
    ADC_ResetCalibration(ADC1);

    while (ADC_GetResetCalibrationStatus(ADC1))
    {
    }

    /*
     * Start calibration.
     */
    ADC_StartCalibration(ADC1);

    while (ADC_GetCalibrationStatus(ADC1))
    {
    }
}


/* ============================================================
 * DMA1 Channel 1
 *
 * ADC1 -> RAM
 *
 * Buffer = 100 samples
 *
 * Half Transfer interrupt
 * Transfer Complete interrupt
 * ============================================================ */

static void DMA1_ADC_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /*
     * Enable DMA1 clock
     */
    RCC_AHBPeriphClockCmd(
        RCC_AHBPeriph_DMA1,
        ENABLE
    );

    /*
     * Disable DMA before configuration.
     */
    DMA_Cmd(
        DMA1_Channel1,
        DISABLE
    );

    /*
     * Reset DMA channel configuration.
     */
    DMA_DeInit(DMA1_Channel1);

    /*
     * DMA configuration
     */
    DMA_InitStructure.DMA_PeripheralBaseAddr =
        (uint32_t)&ADC1->DR;

    DMA_InitStructure.DMA_MemoryBaseAddr =
        (uint32_t)adc_buffer;

    /*
     * ADC -> RAM
     */
    DMA_InitStructure.DMA_DIR =
        DMA_DIR_PeripheralSRC;

    /*
     * 100 samples
     */
    DMA_InitStructure.DMA_BufferSize =
        ADC_BUFFER_SIZE;

    /*
     * ADC data register address does not change.
     */
    DMA_InitStructure.DMA_PeripheralInc =
        DMA_PeripheralInc_Disable;

    /*
     * RAM address increments after every sample.
     */
    DMA_InitStructure.DMA_MemoryInc =
        DMA_MemoryInc_Enable;

    /*
     * ADC data = 16 bit
     */
    DMA_InitStructure.DMA_PeripheralDataSize =
        DMA_PeripheralDataSize_HalfWord;

    /*
     * Buffer = uint16_t
     */
    DMA_InitStructure.DMA_MemoryDataSize =
        DMA_MemoryDataSize_HalfWord;

    /*
     * Circular:
     *
     * 0...99
     * then back to 0...99
     */
    DMA_InitStructure.DMA_Mode =
        DMA_Mode_Circular;

    /*
     * Priority
     */
    DMA_InitStructure.DMA_Priority =
        DMA_Priority_High;

    /*
     * No memory-to-memory.
     */
    DMA_InitStructure.DMA_M2M =
        DMA_M2M_Disable;

    DMA_Init(
        DMA1_Channel1,
        &DMA_InitStructure
    );

    /*
     * Enable Half Transfer interrupt.
     */
    DMA_ITConfig(
        DMA1_Channel1,
        DMA_IT_HT,
        ENABLE
    );

    /*
     * Enable Transfer Complete interrupt.
     */
    DMA_ITConfig(
        DMA1_Channel1,
        DMA_IT_TC,
        ENABLE
    );

    /*
     * NVIC for DMA1 Channel 1.
     */
    NVIC_InitStructure.NVIC_IRQChannel =
        DMA1_Channel1_IRQn;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
        1;

    NVIC_InitStructure.NVIC_IRQChannelSubPriority =
        0;

    NVIC_InitStructure.NVIC_IRQChannelCmd =
        ENABLE;

    NVIC_Init(&NVIC_InitStructure);

    /*
     * Start DMA.
     */
    DMA_Cmd(
        DMA1_Channel1,
        ENABLE
    );
}


/* ============================================================
 * TIM3 configuration
 *
 * Timer frequency = 100 Hz
 *
 * 72 MHz / 7200 = 10 kHz
 *
 * 10 kHz / 100 = 100 Hz
 *
 * TIM3 TRGO = Update event
 *
 * ADC1 uses TIM3 TRGO as external trigger.
 * ============================================================ */

static void TIM3_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    /*
     * Enable TIM3 clock.
     */
    RCC_APB1PeriphClockCmd(
        RCC_APB1Periph_TIM3,
        ENABLE
    );

    /*
     * Prescaler:
     *
     * 72 MHz / (7199 + 1)
     * = 10 kHz
     */
    TIM_TimeBaseStructure.TIM_Prescaler =
        7199;

    /*
     * Up counter.
     */
    TIM_TimeBaseStructure.TIM_CounterMode =
        TIM_CounterMode_Up;

    /*
     * 10 kHz / (99 + 1)
     * = 100 Hz
     */
    TIM_TimeBaseStructure.TIM_Period =
        99;

    /*
     * Clock division not used.
     */
    TIM_TimeBaseStructure.TIM_ClockDivision =
        TIM_CKD_DIV1;

    TIM_TimeBaseStructure.TIM_RepetitionCounter =
        0;

    TIM_TimeBaseInit(
        TIM3,
        &TIM_TimeBaseStructure
    );

    /*
     * TIM3 update event -> TRGO
     */
    TIM_SelectOutputTrigger(
        TIM3,
        TIM_TRGOSource_Update
    );

    /*
     * Start TIM3.
     */
    TIM_Cmd(
        TIM3,
        ENABLE
    );
}


/* ============================================================
 * DMA1 Channel 1 interrupt
 *
 * Half Transfer:
 *     adc_buffer[0 ... 49]
 * is safe to read.
 *
 * Transfer Complete:
 *     adc_buffer[50 ... 99]
 * is safe to read.
 * ============================================================ */

void DMA1_Channel1_IRQHandler(void)
{
    /*
     * Half Transfer
     */
    if (DMA_GetITStatus(DMA1_IT_HT1) != RESET)
    {
        /*
         * Clear HT flag first.
         */
        DMA_ClearITPendingBit(DMA1_IT_HT1);

        /*
         * First half of buffer is complete.
         *
         * DMA is now writing the second half.
         *
         * Therefore [0 ... 49] is safe.
         */
        adc_half_ready = 1;

        /*
         * Send the first 50 samples.
         *
         * The requirement says that when DMA interrupt
         * occurs, transmit the collected data through UART.
         */
        UART1_SendADCBuffer(
            0,
            ADC_BUFFER_SIZE / 2
        );
    }

    /*
     * Transfer Complete
     */
    if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
    {
        /*
         * Clear TC flag first.
         */
        DMA_ClearITPendingBit(DMA1_IT_TC1);

        /*
         * Second half of buffer is complete.
         *
         * DMA is now returning to the first half.
         *
         * Therefore [50 ... 99] is safe.
         */
        adc_full_ready = 1;

        /*
         * Send the second 50 samples.
         */
        UART1_SendADCBuffer(
            ADC_BUFFER_SIZE / 2,
            ADC_BUFFER_SIZE / 2
        );
    }
}


/* ============================================================
 * MAIN
 * ============================================================ */

int main(void)
{
    /*
     * SystemInit() is called before main()
     * by the startup code.
     *
     * System clock = 72 MHz.
     */

    ADC_GPIO_Config();

    UART1_Config();

    /*
     * DMA must be configured before ADC starts
     * producing conversions.
     */
    DMA1_ADC_Config();

    ADC1_Config();

    /*
     * Start Timer last.
     *
     * TIM3 starts producing TRGO events here.
     */
    TIM3_Config();

    UART1_SendString(
        "ADC DMA START\r\n"
    );

    UART1_SendString(
        "Sampling = 100Hz\r\n"
    );

    UART1_SendString(
        "Buffer = 100 samples\r\n"
    );

    /*
     * Main loop.
     *
     * ADC conversion and DMA transfer happen
     * automatically.
     */
    while (1)
    {
        /*
         * Nothing to wait for here.
         *
         * DMA interrupts handle:
         *   - Half Transfer
         *   - Transfer Complete
         */
    }
}
