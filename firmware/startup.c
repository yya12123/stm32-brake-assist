/*
 * startup.c -- 启动文件与中断向量表
 * 复位后：搬运 .data、清零 .bss、调用 SystemInit/main
 */
#include <stdint.h>

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _estack;
extern int main(void);

void SystemInit(void);
void Reset_Handler(void);
void Default_Handler(void);
void SysTick_Handler(void);
void EXTI1_IRQHandler(void);

__attribute__((section(".isr_vector"), used))
void (* const vector_table[])(void) = {
    (void (*)(void))&_estack,       /* 初始栈顶 */
    Reset_Handler,
    Default_Handler,                /* NMI */
    Default_Handler,                /* HardFault */
    Default_Handler,                /* MemManage */
    Default_Handler,                /* BusFault */
    Default_Handler,                /* UsageFault */
    0, 0, 0, 0,                     /* 保留 */
    Default_Handler,                /* SVCall */
    Default_Handler,                /* DebugMon */
    0,                              /* 保留 */
    Default_Handler,                /* PendSV */
    SysTick_Handler,                /* SysTick */
    /* --- IRQ 0..59 --- */
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,  /* 0 WWDG 1 PVD 2 TAMPER 3 RTC */
    Default_Handler, Default_Handler,                                    /* 4 FLASH 5 RCC */
    Default_Handler,                                                     /* 6 EXTI0 */
    EXTI1_IRQHandler,                                                    /* 7 EXTI1  <- 超声波 Echo */
    Default_Handler, Default_Handler, Default_Handler,                   /* 8 EXTI2 9 EXTI3 10 EXTI4 */
    Default_Handler, Default_Handler, Default_Handler, Default_Handler, Default_Handler, Default_Handler, Default_Handler, /* 11..17 DMA1 Ch1..7 */
    Default_Handler, Default_Handler,                                    /* 18 ADC1_2 19 USB_HP */
    Default_Handler, Default_Handler, Default_Handler,                   /* 20 USB_LP 21 CAN_RX1 22 CAN_SCE */
    Default_Handler,                                                     /* 23 EXTI9_5 */
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,  /* 24..27 TIM1 */
    Default_Handler, Default_Handler, Default_Handler,                   /* 28 TIM2 29 TIM3 30 TIM4 */
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,  /* 31..34 I2C */
    Default_Handler, Default_Handler,                                    /* 35 SPI1 36 SPI2 */
    Default_Handler, Default_Handler, Default_Handler,                   /* 37 USART1 38 USART2 39 USART3 */
    Default_Handler,                                                     /* 40 EXTI15_10 */
    Default_Handler, Default_Handler,                                    /* 41 RTCAlarm 42 USBWake */
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,  /* 43..46 TIM8 */
    Default_Handler, Default_Handler, Default_Handler,                   /* 47 ADC3 48 FSMC 49 SDIO */
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,  /* 50 TIM5 51 SPI3 52 UART4 53 UART5 */
    Default_Handler, Default_Handler,                                    /* 54 TIM6 55 TIM7 */
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,  /* 56..59 DMA2 */
};

void Default_Handler(void)
{
    while (1) { }
}


/* ---- 极简 memcpy / memset（-ffreestanding 下不被编译器内联替代）---- */
__attribute__((used))
void *memcpy(void *dst, const void *src, unsigned int n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) { *d++ = *s++; }
    return dst;
}

__attribute__((used))
void *memset(void *dst, int c, unsigned int n)
{
    unsigned char *d = (unsigned char *)dst;
    while (n--) { *d++ = (unsigned char)c; }
    return dst;
}

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) { *dst++ = *src++; }

    dst = &_sbss;
    while (dst < &_ebss) { *dst++ = 0; }

    SystemInit();
    main();
    while (1) { }
}
