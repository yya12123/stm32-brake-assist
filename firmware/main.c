/*
 * main.c -- STM32F103C8T6 电动汽车刹车辅助与灯光预警控制系统（Proteus 仿真固件）
 *
 * 引脚分配：
 *   PA0  ADC1_IN0  制动踏板电位器（0~3.3V -> 0~100%）
 *   PA1  ADC1_IN1  减速度模拟电位器（仿真时代替 MPU6050）
 *   PA2  USART2_TX 虚拟终端
 *   PA3  USART2_RX
 *   PB0  推挽输出  HC-SR04 Trig
 *   PB1  EXTI1     HC-SR04 Echo
 *   PB5  推挽输出  蜂鸣器
 *   PB6  TIM4_CH1  刹车灯 PWM
 *   PB7  推挽输出  左转向/双闪灯
 *   PB8  推挽输出  右转向/双闪灯
 *   PC13 推挽输出  系统运行灯（低电平点亮）
 *   PC14 推挽输出  继电器（模拟制动助力泵）
 */
#include "stm32f103.h"

/* ===================================================================
 * 时钟配置
 *   默认：HSE 8MHz + PLL x9 = 72MHz（与实物一致）
 *   若编译时加 -DRUN_8MHZ，则直接用 8MHz（Proteus 某些版本不模拟 PLL 时使用）
 * =================================================================== */
#ifdef RUN_8MHZ
  #define SYSCLK_HZ   8000000u
  #define APB1_HZ     8000000u
#else
  #define SYSCLK_HZ   72000000u
  #define APB1_HZ     36000000u
#endif

/* ===================== 时钟与延时 ===================== */
volatile uint32_t g_ms = 0;

void SysTick_Handler(void)
{
    g_ms++;
}

void SystemInit(void)
{
    /* 1. 打开外部 8MHz 晶振（若没有晶振则自动退回内部 8MHz RC） */
    RCC_CR |= RCC_CR_HSEON;
    {
        volatile uint32_t n = 200000u;
        while (!(RCC_CR & RCC_CR_HSERDY) && --n) { }
        if (!(RCC_CR & RCC_CR_HSERDY)) {
            RCC_CFGR = (0u << 4) | (0u << 8) | (0u << 11) | (2u << 14);
            REG32(0xE000ED08) = 0x08000000u;
            SYST_RVR = 8000u - 1u;
            SYST_CVR = 0;
            SYST_CSR = 7u;
            return;             /* 内部 8MHz 运行 */
        }
    }

#ifdef RUN_8MHZ
    /* 8MHz 直跑：不分频、不用 PLL */
    RCC_CFGR = (0u << 4) | (0u << 8) | (0u << 11) | (2u << 14);
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW) | 1u;      /* 系统时钟 = HSE */
    while ((RCC_CFGR & RCC_CFGR_SWS) != (1u << 2)) { }

    REG32(0xE000ED08) = 0x08000000u;
    SYST_RVR = 8000u - 1u;
    SYST_CVR = 0;
    SYST_CSR = 7u;
    return;
#endif

    /* 2. Flash 等待周期：72MHz 需要 2 个等待周期 */
    FLASH_ACR = 0x12;

    /* 3. 总线分频：AHB=72M, APB2=72M, APB1=36M；ADC 预分频 /6 = 12MHz */
    RCC_CFGR = (0u << 4) | (4u << 8) | (0u << 11) | (2u << 14);

    /* 4. PLL：HSE(8M) x9 = 72MHz */
    RCC_CFGR |= RCC_CFGR_PLLSRC | (7u << 18);
    RCC_CR |= RCC_CR_PLLON;
    while (!(RCC_CR & RCC_CR_PLLRDY)) { }

    /* 5. 切换系统时钟到 PLL */
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW) | 2u;
    while ((RCC_CFGR & RCC_CFGR_SWS) != (2u << 2)) { }

    /* 6. 中断向量表基地址指向 Flash */
    REG32(0xE000ED08) = 0x08000000u;

    /* 7. SysTick 1ms */
    SYST_RVR = (SYSCLK_HZ / 1000u) - 1u;
    SYST_CVR = 0;
    SYST_CSR = 7u;
}

void delay_ms(uint32_t ms)
{
    uint32_t t = g_ms + ms;
    while ((int32_t)(g_ms - t) < 0) { }
}

void delay_us(uint32_t us)
{
    volatile uint32_t n = us * 8u;
    while (n--) { __asm volatile ("nop"); }
}

/* ===================== 串口 ===================== */
void uart_init(void)
{
    RCC_APB2ENR |= RCC_APB2_IOPA;
    RCC_APB1ENR |= RCC_APB1_USART2;

    /* PA2 = 复用推挽输出 50MHz，PA3 = 浮空输入 */
    GPIOA_CRL = (GPIOA_CRL & ~(0xFu << 8)) | (0xBu << 8);
    GPIOA_CRL = (GPIOA_CRL & ~(0xFu << 12)) | (0x4u << 12);

    USART2_BRR = APB1_HZ / 9600u;        /* APB1 = 36MHz(72M配置) / 8MHz(8M配置) */
    USART2_CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void uart_putc(char c)
{
    while (!(USART2_SR & USART_SR_TXE)) { }
    USART2_DR = (uint8_t)c;
}

void uart_puts(const char *s)
{
    while (*s) { uart_putc(*s++); }
}

void uart_num(int32_t v)
{
    char buf[12];
    int i = 0;
    if (v < 0) { uart_putc('-'); v = -v; }
    if (v == 0) { uart_putc('0'); return; }
    while (v > 0) { buf[i++] = (char)('0' + (v % 10)); v /= 10; }
    while (i--) { uart_putc(buf[i]); }
}

/* ===================== ADC ===================== */
void adc_init(void)
{
    RCC_APB2ENR |= RCC_APB2_ADC1;

    /* PA0 / PA1 模拟输入（CNF=00, MODE=00 => 0x0） */
    GPIOA_CRL &= ~((0xFu << 0) | (0xFu << 4));

    /* 采样时间 55.5 周期 */
    ADC1_SMPR2 |= (5u << 0) | (5u << 3);

    ADC1_CR2 = ADC_CR2_ADON;
    delay_us(10);
    ADC1_CR2 |= ADC_CR2_RSTCAL;
    while (ADC1_CR2 & ADC_CR2_RSTCAL) { }
    ADC1_CR2 |= ADC_CR2_CAL;
    while (ADC1_CR2 & ADC_CR2_CAL) { }
    ADC1_CR2 |= ADC_CR2_ADON;
}

uint16_t adc_read(uint8_t ch)
{
    ADC1_SQR3 = ch;
    ADC1_CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1_SR & ADC_SR_EOC)) { }
    return (uint16_t)ADC1_DR;
}

/* ===================== 刹车灯 PWM ===================== */
void pwm_init(void)
{
    RCC_APB2ENR |= RCC_APB2_IOPB;
    RCC_APB1ENR |= RCC_APB1_TIM4;

    /* PB6 = 复用推挽 50MHz */
    GPIOB_CRL = (GPIOB_CRL & ~(0xFu << 24)) | (0xBu << 24);

    TIM4_PSC = (SYSCLK_HZ / 1000000u) - 1u;   /* 计数频率 1MHz */
    TIM4_ARR = 999;         /* 1kHz PWM */
    TIM4_CCMR1 = (6u << 4) | TIM_CCMR1_OC1PE;   /* PWM 模式 1 */
    TIM4_CCER |= TIM_CCER_CC1E;
    TIM4_EGR = TIM_EGR_UG;
    TIM4_CR1 = TIM_CR1_CEN | TIM_CR1_ARPE;
}

void pwm_set(uint16_t duty)      /* 0 ~ 1000 */
{
    if (duty > 999) duty = 999;
    TIM4_CCR1 = duty;
}

/* ===================== 普通 GPIO ===================== */
void gpio_init(void)
{
    RCC_APB2ENR |= RCC_APB2_IOPA | RCC_APB2_IOPB | RCC_APB2_IOPC | RCC_APB2_AFIO;

    /* PB0(Trig) PB5(蜂鸣器) PB7 PB8 : 推挽输出 2MHz */
    GPIOB_CRL = (GPIOB_CRL & ~(0xFu << 0))  | (0x2u << 0);
    GPIOB_CRL = (GPIOB_CRL & ~(0xFu << 20)) | (0x2u << 20);
    GPIOB_CRL = (GPIOB_CRL & ~(0xFu << 28)) | (0x2u << 28);   /* PB7 (CRL 的 31:28 位) */
    GPIOB_CRH = (GPIOB_CRH & ~(0xFu << 0))  | (0x2u << 0);    /* PB8 */

    /* PB1 = 上拉输入（Echo）：万一模型是开漏输出，没有上拉会一直读低 */
    GPIOB_CRL = (GPIOB_CRL & ~(0xFu << 4)) | (0x8u << 4);
    GPIOB_BSRR = (1u << 1);          /* ODR1=1 才真正使能内部上拉 */

    /* PC13 运行灯、PC14 继电器：推挽输出 2MHz */
    GPIOC_CRH = (GPIOC_CRH & ~(0xFu << 20)) | (0x2u << 20);
    GPIOC_CRH = (GPIOC_CRH & ~(0xFu << 24)) | (0x2u << 24);

    GPIOB_BSRR = (1u << 16) | (1u << (5 + 16)) | (1u << (7 + 16)) | (1u << (8 + 16));
    GPIOC_BSRR = (1u << (14 + 16));
    GPIOB_BSRR = (1u << 0) << 16;   /* Trig 低 */
}

/* ===================== 超声波 HC-SR04 ===================== */
volatile uint32_t echo_width = 0;
volatile uint8_t  echo_done = 0;

void EXTI1_IRQHandler(void)
{
    if (EXTI_PR & (1u << 1)) {
        EXTI_PR = (1u << 1);
        if (GPIOB_IDR & (1u << 1)) {        /* 上升沿 */
            TIM2_CNT = 0;
            echo_done = 0;
        } else {                            /* 下降沿 */
            echo_width = TIM2_CNT;
            echo_done = 1;
        }
    }
}

void hcsr04_init(void)
{
    RCC_APB1ENR |= RCC_APB1_TIM2;
    RCC_APB2ENR |= RCC_APB2_AFIO;

    /* TIM2：1MHz 自由计数 */
    TIM2_PSC = (SYSCLK_HZ / 1000000u) - 1u;
    TIM2_ARR = 0xFFFF;
    TIM2_EGR = TIM_EGR_UG;
    TIM2_CR1 = TIM_CR1_CEN;

    /* EXTI1 映射到 PB1，双边沿触发 */
    AFIO_EXTICR1 = (AFIO_EXTICR1 & ~(0xFu << 4)) | (0x1u << 4);
    EXTI_IMR |= (1u << 1);
    EXTI_RTSR |= (1u << 1);
    EXTI_FTSR |= (1u << 1);

    /* 使能 EXTI1 中断（IRQ 号 = 7） */
    REG32(0xE000E404) = 0xC0000000u;        /* EXTI1（IRQ7）抢占优先级 3 */
    NVIC_ISER = (1u << 7);
}

uint16_t hcsr04_measure_cm(void)
{
    uint32_t timeout;

    uint32_t t0, t1;

    /* 1. 发 20us 触发脉冲 */
    GPIOB_BSRR = (1u << 0);
    delay_us(20);
    GPIOB_BSRR = (1u << (0 + 16));
    TIM2_CNT = 0;

    /* 2. 轮询等待回波上升沿 */
    timeout = g_ms + 35u;
    while (!(GPIOB_IDR & (1u << 1))) {
        if ((int32_t)(g_ms - timeout) >= 0) { echo_width = 0; return 0xFFFF; }
    }
    t0 = TIM2_CNT;

    /* 3. 轮询等待回波下降沿 */
    timeout = g_ms + 35u;
    while ((GPIOB_IDR & (1u << 1))) {
        if ((int32_t)(g_ms - timeout) >= 0) { echo_width = 0; return 0xFFFF; }
    }
    t1 = TIM2_CNT;

    echo_width = t1 - t0;
#ifdef SIM_CAL
    /* 仿真标定：Proteus SRF04 模型按 300m/s 声速产生回波(约 66.4 us/cm)，
       实物 HC-SR04 用 58us/cm。编译时加 -DSIM_CAL 启用仿真系数。 */
    return (uint16_t)(echo_width / 66u);
#else
    return (uint16_t)(echo_width / 58u);
#endif
}

/* ===================== 控制逻辑 ===================== */
typedef enum {
    ST_NORMAL = 0,
    ST_FOLLOW_WARN,
    ST_FOLLOW_URGENT,
    ST_BRAKE_ASSIST
} sys_state_t;

static uint16_t pedal_filt = 0;
static uint8_t  pedal_pct = 0;
static int16_t  pedal_rate = 0;
static uint8_t  decel_pct = 0;
static uint16_t dist_cm = 0;
static sys_state_t state = ST_NORMAL;
static uint32_t assist_timer = 0;
static uint32_t blink_phase = 0;
static uint8_t  left_on = 0, right_on = 0;
static uint8_t  brake_duty_pct = 0;

int main(void)
{
    uint16_t raw;
    uint32_t tick_next_measure = 0;
    uint32_t tick_next_report = 0;
    uint32_t tick_next_blink = 0;
    uint32_t tick_next_display = 0;

    gpio_init();
    uart_init();
    adc_init();
    pwm_init();
    hcsr04_init();

    uart_puts("\r\n=== Brake Assist & Light Warning System ===\r\n");
    uart_puts("STM32F103C8T6 @");
#ifdef RUN_8MHZ
    uart_puts("8MHz");
#else
    uart_puts("72MHz");
#endif
    uart_puts(", 9600bps\r\n");
    uart_puts("P:pedal% R:rate D:decel% V:distance(cm) S:state\r\n");

    while (1) {
        /* ---- 10ms 周期任务 ---- */
        raw = adc_read(0);
        pedal_filt = (uint16_t)((pedal_filt * 7u + raw) / 8u);   /* 一阶滤波 */

        {
            uint8_t new_pct = (uint8_t)((uint32_t)pedal_filt * 100u / 4095u);
            pedal_rate = (int16_t)((int16_t)new_pct - (int16_t)pedal_pct);
            pedal_pct = new_pct;
        }

        decel_pct = (uint8_t)((uint32_t)adc_read(1) * 100u / 4095u);

        /* ---- 刹车灯三级调光 ---- */
        if (pedal_pct < 5)        brake_duty_pct = 0;
        else if (pedal_pct < 40)  brake_duty_pct = 25;
        else if (pedal_pct < 70)  brake_duty_pct = 50;
        else                      brake_duty_pct = 90;
        pwm_set((uint16_t)brake_duty_pct * 10u);

        /* ---- 紧急制动判定：踏板变化率 + 减速度 双阈值 ---- */
        if (pedal_rate >= 6 && decel_pct >= 50) {
            state = ST_BRAKE_ASSIST;
            assist_timer = g_ms;
        }
        if (state == ST_BRAKE_ASSIST && (g_ms - assist_timer) > 2000u) {
            state = ST_NORMAL;
        }

        /* ---- 每 100ms 测一次距离 ---- */
        if ((int32_t)(g_ms - tick_next_measure) >= 0) {
            tick_next_measure = g_ms + 100u;
            dist_cm = hcsr04_measure_cm();
            if (dist_cm < 50)                     state = ST_FOLLOW_URGENT;
            else if (dist_cm < 150)               state = ST_FOLLOW_WARN;
            else if (state != ST_BRAKE_ASSIST)    state = ST_NORMAL;
        }

        /* ---- 灯光：双闪 1Hz / 跟车警告闪烁 ---- */
        if ((int32_t)(g_ms - tick_next_blink) >= 0) {
            if (state == ST_BRAKE_ASSIST) {
                tick_next_blink = g_ms + 250u;
                left_on = right_on = (uint8_t)(blink_phase ^ 1u);
                blink_phase ^= 1u;
            } else if (state == ST_FOLLOW_URGENT) {
                tick_next_blink = g_ms + 100u;
                left_on = right_on = (uint8_t)(blink_phase ^ 1u);
                blink_phase ^= 1u;
            } else if (state == ST_FOLLOW_WARN) {
                tick_next_blink = g_ms + 300u;
                left_on = right_on = (uint8_t)(blink_phase ^ 1u);
                blink_phase ^= 1u;
            } else {
                tick_next_blink = g_ms + 100u;
                left_on = right_on = 0;
            }
        }
        BIT_WRITE(GPIOB_BSRR, 7, left_on);
        BIT_WRITE(GPIOB_BSRR, 8, right_on);

        /* ---- 蜂鸣器与继电器 ---- */
        BIT_WRITE(GPIOB_BSRR, 5, (state == ST_FOLLOW_URGENT || state == ST_BRAKE_ASSIST) ? 1 : 0);
        BIT_WRITE(GPIOC_BSRR, 14, (state == ST_BRAKE_ASSIST) ? 1 : 0);

        /* ---- 运行灯 500ms 翻转 ---- */
        if ((g_ms % 1000u) < 500u) {
            GPIOC_BSRR = (1u << (13 + 16));     /* PC13 拉低 -> 灯亮 500ms */
        } else {
            GPIOC_BSRR = (1u << 13);            /* PC13 拉高 -> 灯灭 500ms */
        }

        /* ---- 串口上报 10Hz ---- */
        if ((int32_t)(g_ms - tick_next_report) >= 0) {
            tick_next_report = g_ms + 100u;
            uart_puts("P="); uart_num(pedal_pct);
            uart_puts(" R="); uart_num(pedal_rate);
            uart_puts(" D="); uart_num(decel_pct);
            uart_puts(" V="); uart_num((int32_t)dist_cm);
            uart_puts(" W="); uart_num((int32_t)echo_width);
            uart_puts(" S="); uart_num(state);
            uart_puts("\r\n");
        }

        (void)tick_next_display;
        delay_ms(10);
    }
}
