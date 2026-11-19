/*
 * stm32f103.h  --  STM32F103C8T6 寄存器定义（纯寄存器编程，不依赖任何库）
 * 用于「电动汽车刹车辅助与灯光预警系统」固件
 * 编译: arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb
 */
#ifndef STM32F103_H
#define STM32F103_H

#include <stdint.h>

#define REG32(a)        (*(volatile uint32_t *)(a))
#define REG16(a)        (*(volatile uint16_t *)(a))

/* ---------------- 时钟 RCC ---------------- */
#define RCC_CR          REG32(0x40021000)
#define RCC_CFGR        REG32(0x40021004)
#define RCC_APB2ENR     REG32(0x40021018)
#define RCC_APB1ENR     REG32(0x4002101C)

#define RCC_CR_HSEON    (1u << 16)
#define RCC_CR_HSERDY   (1u << 17)
#define RCC_CR_PLLON    (1u << 24)
#define RCC_CR_PLLRDY   (1u << 25)
#define RCC_CFGR_SW     (3u << 0)
#define RCC_CFGR_SWS    (3u << 2)
#define RCC_CFGR_PLLSRC (1u << 16)
#define RCC_CFGR_PLLMUL (0xFu << 18)
#define RCC_CFGR_PLLXTPRE (1u << 17)
#define RCC_CFGR_PPRE1  (7u << 8)
#define RCC_CFGR_PPRE2  (7u << 11)
#define RCC_CFGR_HPRE   (0xFu << 4)

#define RCC_APB2_AFIO   (1u << 0)
#define RCC_APB2_IOPA   (1u << 2)
#define RCC_APB2_IOPB   (1u << 3)
#define RCC_APB2_IOPC   (1u << 4)
#define RCC_APB2_ADC1   (1u << 9)
#define RCC_APB1_TIM2   (1u << 0)
#define RCC_APB1_TIM4   (1u << 2)
#define RCC_APB1_USART2 (1u << 17)

/* ---------------- Flash ---------------- */
#define FLASH_ACR       REG32(0x40022000)

/* ---------------- GPIO ---------------- */
#define GPIOA_CRL       REG32(0x40010800)
#define GPIOA_CRH       REG32(0x40010804)
#define GPIOA_IDR       REG32(0x40010808)
#define GPIOA_ODR       REG32(0x4001080C)
#define GPIOA_BSRR      REG32(0x40010810)
#define GPIOB_CRL       REG32(0x40010C00)
#define GPIOB_CRH       REG32(0x40010C04)
#define GPIOB_IDR       REG32(0x40010C08)
#define GPIOB_ODR       REG32(0x40010C0C)
#define GPIOB_BSRR      REG32(0x40010C10)
#define GPIOC_CRL       REG32(0x40011000)
#define GPIOC_CRH       REG32(0x40011004)
#define GPIOC_IDR       REG32(0x40011008)
#define GPIOC_ODR       REG32(0x4001100C)
#define GPIOC_BSRR      REG32(0x40011010)

/* ---------------- 外部中断 EXTI / AFIO ---------------- */
#define AFIO_EXTICR1    REG32(0x40010008)
#define AFIO_EXTICR2    REG32(0x4001000C)
#define EXTI_IMR        REG32(0x40010400)
#define EXTI_EMR        REG32(0x40010404)
#define EXTI_RTSR       REG32(0x40010408)
#define EXTI_FTSR       REG32(0x4001040C)
#define EXTI_PR         REG32(0x40010414)

/* ---------------- ADC1 ---------------- */
#define ADC1_SR         REG32(0x40012400)
#define ADC1_CR1        REG32(0x40012404)
#define ADC1_CR2        REG32(0x40012408)
#define ADC1_SMPR1      REG32(0x4001240C)
#define ADC1_SMPR2      REG32(0x40012410)
#define ADC1_SQR1       REG32(0x4001242C)
#define ADC1_SQR3       REG32(0x40012434)
#define ADC1_DR         REG32(0x4001244C)

#define ADC_CR2_ADON    (1u << 0)
#define ADC_CR2_CONT    (1u << 1)
#define ADC_CR2_CAL     (1u << 2)
#define ADC_CR2_RSTCAL  (1u << 3)
#define ADC_CR2_EXTSEL  (0x7u << 17)
#define ADC_CR2_SWSTART (1u << 22)
#define ADC_SR_EOC      (1u << 1)
#define ADC_SR_STRT     (1u << 4)

/* ---------------- 定时器 TIM2 / TIM4 ---------------- */
#define TIM2_CR1        REG32(0x40000000)
#define TIM2_DIER       REG32(0x4000000C)
#define TIM2_SR         REG32(0x40000010)
#define TIM2_EGR        REG32(0x40000014)
#define TIM2_CNT        REG32(0x40000024)
#define TIM2_PSC        REG32(0x40000028)
#define TIM2_ARR        REG32(0x4000002C)

#define TIM4_CR1        REG32(0x40000800)
#define TIM4_DIER       REG32(0x4000080C)
#define TIM4_SR         REG32(0x40000810)
#define TIM4_EGR        REG32(0x40000814)
#define TIM4_CCMR1      REG32(0x40000818)
#define TIM4_CCER       REG32(0x40000820)
#define TIM4_CNT        REG32(0x40000824)
#define TIM4_PSC        REG32(0x40000828)
#define TIM4_ARR        REG32(0x4000082C)
#define TIM4_CCR1       REG32(0x40000834)

#define TIM_CR1_CEN     (1u << 0)
#define TIM_CR1_ARPE    (1u << 7)
#define TIM_EGR_UG      (1u << 0)
#define TIM_CCER_CC1E   (1u << 0)
#define TIM_CCMR1_OC1M  (0x7u << 4)
#define TIM_CCMR1_OC1PE (1u << 3)

/* ---------------- USART2 ---------------- */
#define USART2_SR       REG32(0x40004400)
#define USART2_DR       REG32(0x40004404)
#define USART2_BRR      REG32(0x40004408)
#define USART2_CR1      REG32(0x4000440C)
#define USART_SR_TXE    (1u << 7)
#define USART_SR_RXNE   (1u << 5)
#define USART_SR_TC     (1u << 6)
#define USART_CR1_UE    (1u << 13)
#define USART_CR1_TE    (1u << 3)
#define USART_CR1_RE    (1u << 2)

/* ---------------- NVIC / SysTick ---------------- */
#define NVIC_ISER       REG32(0xE000E100)
#define NVIC_ICER       REG32(0xE000E180)
#define NVIC_IPR        REG32(0xE000E400)
#define SYST_CSR        REG32(0xE000E010)
#define SYST_RVR        REG32(0xE000E014)
#define SYST_CVR        REG32(0xE000E018)

#define EXTI1_IRQn      7
#define EXTI9_5_IRQn    23
#define EXTI15_10_IRQn  40

/* 简易位操作 */
#define BIT_SET(reg, bit)     ((reg) = (1u << (bit)))
#define BIT_CLR(reg, bit)     ((reg) = (1u << ((bit) + 16)))
#define BIT_WRITE(reg, bit, v) ((v) ? BIT_SET(reg, bit) : BIT_CLR(reg, bit))

#endif
