#include "stm32f10x.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* =========================================================
 * USER CONFIGURATION
 * ========================================================= */

#define ID_LOP   "D23CQDT04"
#define ID_NHOM  "07"

/* Nút nhấn PA0 */
#define BUTTON_PORT        GPIOA
#define BUTTON_PIN         GPIO_Pin_0

/* Kích thước queue */
#define EVENT_QUEUE_SIZE   16


/* =========================================================
 * GLOBAL VARIABLES
 * ========================================================= */

/* Giá trị tăng mỗi lần nhấn nút */
volatile uint32_t button_value = 0;

/* Queue lưu các sự kiện nút nhấn */
volatile uint32_t event_queue[EVENT_QUEUE_SIZE];

volatile uint8_t queue_head = 0;
volatile uint8_t queue_tail = 0;

/* DMA đang truyền = 1 */
volatile uint8_t dma_tx_busy = 0;

/* Bộ đếm thời gian ms */
volatile uint32_t system_ms = 0;

/* Thời điểm nhấn nút gần nhất */
volatile uint32_t last_button_time = 0;

/* Buffer truyền DMA */
static char tx_buffer[64];


/* =========================================================
 * SYSTICK - 1ms
 * ========================================================= */

static void SysTick_Config_1ms(void)
{
    /*
     * SystemCoreClock = 72 MHz
     *
     * 72,000,000 / 1000
     * = 72,000 lần đếm / 1 ms
     *
     * SysTick_Config() tự động:
     * - thiết lập reload
     * - bật interrupt
     * - bật counter
     */
    SysTick_Config(SystemCoreClock / 1000);
}


/* =========================================================
 * SYSTICK INTERRUPT
 * ========================================================= */

void SysTick_Handler(void)
{
    system_ms++;
}


/* =========================================================
 * UART1 CONFIGURATION
 * ========================================================= */

static void UART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /*
     * Enable clock:
     * GPIOA
     * USART1
     */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_USART1,
        ENABLE
    );


    /* -----------------------------------------------------
     * PA9 = USART1_TX
     * ----------------------------------------------------- */

    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_9;

    GPIO_InitStructure.GPIO_Mode =
        GPIO_Mode_AF_PP;

    GPIO_InitStructure.GPIO_Speed =
        GPIO_Speed_50MHz;

    GPIO_Init(
        GPIOA,
        &GPIO_InitStructure
    );


    /* -----------------------------------------------------
     * PA10 = USART1_RX
     * ----------------------------------------------------- */

    GPIO_InitStructure.GPIO_Pin =
        GPIO_Pin_10;

    GPIO_InitStructure.GPIO_Mode =
        GPIO_Mode_IN_FLOATING;

    GPIO_Init(
        GPIOA,
        &GPIO_InitStructure
    );


    /* -----------------------------------------------------
     * USART1
     * ----------------------------------------------------- */

    USART_InitStructure.USART_BaudRate =
        115200;

    USART_InitStructure.USART_WordLength =
        USART_WordLength_8b;

    USART_InitStructure.USART_StopBits =
        USART_StopBits_1;

    USART_InitStructure.USART_Parity =
        USART_Parity_No;

    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;

    USART_InitStructure.USART_Mode =
        USART_Mode_Tx |
        USART_Mode_Rx;

    USART_Init(
        USART1,
        &USART_InitStructure
    );

    USART_Cmd(
        USART1,
        ENABLE
    );


    /* -----------------------------------------------------
     * NVIC DMA1 Channel 4
     * ----------------------------------------------------- */

    NVIC_InitStructure.NVIC_IRQChannel =
        DMA1_Channel4_IRQn;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
        1;

    NVIC_InitStructure.NVIC_IRQChannelSubPriority =
        1;

    NVIC_InitStructure.NVIC_IRQChannelCmd =
        ENABLE;

    NVIC_Init(
        &NVIC_InitStructure
    );
}


/* =========================================================
 * DMA USART1 TX CONFIGURATION
 * ========================================================= */

static void DMA_USART1_TX_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    /*
     * Enable DMA1 clock
     */
    RCC_AHBPeriphClockCmd(
        RCC_AHBPeriph_DMA1,
        ENABLE
    );


    /*
     * Disable DMA before configuration
     */
    DMA_Cmd(
        DMA1_Channel4,
        DISABLE
    );

    DMA_DeInit(
        DMA1_Channel4
    );


    /*
     * Peripheral:
     * USART1 data register
     */
    DMA_InitStructure.DMA_PeripheralBaseAddr =
        (uint32_t)&USART1->DR;


    /*
     * Memory:
     * transmit buffer
     */
    DMA_InitStructure.DMA_MemoryBaseAddr =
        (uint32_t)tx_buffer;


    /*
     * Direction:
     * Memory -> Peripheral
     */
    DMA_InitStructure.DMA_DIR =
        DMA_DIR_PeripheralDST;


    /*
     * Số byte sẽ được cập nhật
     * trước mỗi lần truyền.
     */
    DMA_InitStructure.DMA_BufferSize =
        0;


    /*
     * USART1->DR cố định
     */
    DMA_InitStructure.DMA_PeripheralInc =
        DMA_PeripheralInc_Disable;


    /*
     * Buffer tăng địa chỉ
     */
    DMA_InitStructure.DMA_MemoryInc =
        DMA_MemoryInc_Enable;


    /*
     * Data size = 8 bit
     */
    DMA_InitStructure.DMA_PeripheralDataSize =
        DMA_PeripheralDataSize_Byte;

    DMA_InitStructure.DMA_MemoryDataSize =
        DMA_MemoryDataSize_Byte;


    /*
     * Normal mode:
     * truyền xong thì dừng.
     */
    DMA_InitStructure.DMA_Mode =
        DMA_Mode_Normal;


    /*
     * Priority cao
     */
    DMA_InitStructure.DMA_Priority =
        DMA_Priority_High;


    /*
     * Không dùng Memory-to-Memory
     */
    DMA_InitStructure.DMA_M2M =
        DMA_M2M_Disable;


    DMA_Init(
        DMA1_Channel4,
        &DMA_InitStructure
    );


    /*
     * Enable Transfer Complete interrupt
     */
    DMA_ITConfig(
        DMA1_Channel4,
        DMA_IT_TC,
        ENABLE
    );


    /*
     * Cho phép USART1 phát yêu cầu DMA
     */
    USART_DMACmd(
        USART1,
        USART_DMAReq_Tx,
        ENABLE
    );
}


/* =========================================================
 * BUTTON + EXTI CONFIGURATION
 * ========================================================= */

static void Button_EXTI_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;


    /*
     * Enable:
     * GPIOA
     * AFIO
     */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_AFIO,
        ENABLE
    );


    /* -----------------------------------------------------
     * PA0 = Input Pull-up
     *
     * Không nhấn = 1
     * Nhấn       = 0
     * ----------------------------------------------------- */

    GPIO_InitStructure.GPIO_Pin =
        BUTTON_PIN;

    GPIO_InitStructure.GPIO_Mode =
        GPIO_Mode_IPU;

    GPIO_InitStructure.GPIO_Speed =
        GPIO_Speed_2MHz;

    GPIO_Init(
        BUTTON_PORT,
        &GPIO_InitStructure
    );


    /* -----------------------------------------------------
     * PA0 -> EXTI0
     * ----------------------------------------------------- */

    GPIO_EXTILineConfig(
        GPIO_PortSourceGPIOA,
        GPIO_PinSource0
    );


    /* -----------------------------------------------------
     * EXTI0
     * ----------------------------------------------------- */

    EXTI_InitStructure.EXTI_Line =
        EXTI_Line0;

    EXTI_InitStructure.EXTI_Mode =
        EXTI_Mode_Interrupt;

    /*
     * Phát hiện cạnh xuống:
     *
     * 1 -> 0
     */
    EXTI_InitStructure.EXTI_Trigger =
        EXTI_Trigger_Falling;

    EXTI_InitStructure.EXTI_LineCmd =
        ENABLE;

    EXTI_Init(
        &EXTI_InitStructure
    );


    /* -----------------------------------------------------
     * NVIC EXTI0
     * ----------------------------------------------------- */

    NVIC_InitStructure.NVIC_IRQChannel =
        EXTI0_IRQn;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
        0;

    NVIC_InitStructure.NVIC_IRQChannelSubPriority =
        0;

    NVIC_InitStructure.NVIC_IRQChannelCmd =
        ENABLE;

    NVIC_Init(
        &NVIC_InitStructure
    );
}


/* =========================================================
 * QUEUE PUSH
 * ========================================================= */

static void Queue_Push(uint32_t value)
{
    uint8_t next_head;

    next_head =
        (uint8_t)(
            (queue_head + 1) %
            EVENT_QUEUE_SIZE
        );


    /*
     * Nếu queue chưa đầy
     */
    if (next_head != queue_tail)
    {
        event_queue[queue_head] =
            value;

        queue_head =
            next_head;
    }
}


/* =========================================================
 * QUEUE POP
 * ========================================================= */

static uint8_t Queue_Pop(uint32_t *value)
{
    /*
     * Queue rỗng
     */
    if (queue_tail == queue_head)
    {
        return 0;
    }


    /*
     * Lấy giá trị
     */
    *value =
        event_queue[queue_tail];


    /*
     * Tăng vị trí đọc
     */
    queue_tail =
        (uint8_t)(
            (queue_tail + 1) %
            EVENT_QUEUE_SIZE
        );


    return 1;
}


/* =========================================================
 * UART DMA SEND
 * ========================================================= */

static void UART_DMA_SendValue(uint32_t value)
{
    uint16_t length;


    /*
     * Tạo bản tin:
     *
     * D23CQDT0407:BTN:1\n\r
     */
    snprintf(
        tx_buffer,
        sizeof(tx_buffer),
        "%s%s:BTN:%lu\n\r",
        ID_LOP,
        ID_NHOM,
        (unsigned long)value
    );


    /*
     * Độ dài bản tin
     */
    length =
        (uint16_t)strlen(
            tx_buffer
        );


    /*
     * DMA đang bận
     */
    dma_tx_busy = 1;


    /*
     * Tắt DMA trước khi
     * thay đổi cấu hình
     */
    DMA_Cmd(
        DMA1_Channel4,
        DISABLE
    );


    /*
     * Xóa các cờ DMA
     */
    DMA_ClearFlag(
        DMA1_FLAG_GL4 |
        DMA1_FLAG_TC4 |
        DMA1_FLAG_HT4 |
        DMA1_FLAG_TE4
    );


    /*
     * Địa chỉ buffer
     */
    DMA1_Channel4->CMAR =
        (uint32_t)tx_buffer;


    /*
     * Số byte truyền
     */
    DMA1_Channel4->CNDTR =
        length;


    /*
     * Bật DMA
     */
    DMA_Cmd(
        DMA1_Channel4,
        ENABLE
    );
}


/* =========================================================
 * DMA1 CHANNEL 4 INTERRUPT
 * ========================================================= */

void DMA1_Channel4_IRQHandler(void)
{
    if (
        DMA_GetITStatus(
            DMA1_IT_TC4
        ) != RESET
    )
    {
        /*
         * Xóa cờ Transfer Complete
         */
        DMA_ClearITPendingBit(
            DMA1_IT_TC4
        );


        /*
         * Dừng DMA
         */
        DMA_Cmd(
            DMA1_Channel4,
            DISABLE
        );


        /*
         * DMA đã truyền xong buffer.
         *
         * CPU không phải chờ
         * USART_FLAG_TXE hoặc
         * USART_FLAG_TC.
         */
        dma_tx_busy = 0;
    }
}


/* =========================================================
 * EXTI0 INTERRUPT
 * ========================================================= */

void EXTI0_IRQHandler(void)
{
    if (
        EXTI_GetITStatus(
            EXTI_Line0
        ) != RESET
    )
    {
        /*
         * Chống dội nút 50 ms
         */
        if (
            (system_ms - last_button_time)
            >= 50
        )
        {
            /*
             * Lưu thời điểm nhấn
             */
            last_button_time =
                system_ms;


            /*
             * Tăng giá trị
             */
            button_value++;


            /*
             * Đưa giá trị vào queue
             */
            Queue_Push(
                button_value
            );
        }


        /*
         * Xóa cờ EXTI
         */
        EXTI_ClearITPendingBit(
            EXTI_Line0
        );
    }
}


/* =========================================================
 * MAIN
 * ========================================================= */

int main(void)
{
    uint32_t value_to_send;


    /*
     * SystemInit() được gọi trước main()
     * bởi startup code.
     *
     * SystemCoreClock = 72 MHz.
     */


    /*
     * SysTick = 1 ms
     */
    SysTick_Config_1ms();


    /*
     * UART1
     */
    UART1_Config();


    /*
     * DMA USART1 TX
     */
    DMA_USART1_TX_Config();


    /*
     * Button PA0 + EXTI0
     */
    Button_EXTI_Config();


    /*
     * Main loop
     *
     * KHÔNG sử dụng hàm chờ UART.
     */
    while (1)
    {
        /*
         * DMA đang rảnh
         */
        if (dma_tx_busy == 0)
        {
            /*
             * Có sự kiện nút nhấn
             */
            if (
                Queue_Pop(
                    &value_to_send
                )
            )
            {
                /*
                 * Bắt đầu truyền bằng DMA
                 */
                UART_DMA_SendValue(
                    value_to_send
                );
            }
        }
    }
}
