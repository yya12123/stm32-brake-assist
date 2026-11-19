# STM32 缩比电动车 · 刹车辅助与灯光预警系统

**简体中文** · [English](README.en.md)

> STM32F103C8T6 · Proteus 仿真 + 实物缩比小车

踩下制动踏板，系统按踏板开度分级点亮刹车灯；识别到急刹意图时自动双闪、鸣笛，并驱动继电器模拟制动助力泵；同时用超声波持续监测前方车距，进入跟车预警或危险报警状态。全部逻辑跑在 STM32F103C8T6 上，裸机、直接写寄存器、无 HAL 依赖。

## 功能

| 模块 | 行为 |
| --- | --- |
| 制动灯调光 | 踏板开度 <5% 熄灭、<40% 25%、<70% 50%、≥70% 90%（TIM4_CH1 PWM） |
| 紧急制动辅助 | 踏板变化率 ≥6%/10ms 且减速度 ≥50% → 制动辅助 2s：双闪 1Hz、蜂鸣器、继电器吸合 |
| 跟车预警 | 距离 <150cm 双黄灯 300ms 闪烁；<50cm 100ms 急闪 + 蜂鸣器 |
| 状态上报 | USART2 9600bps，10Hz 输出 `P R D V W S` |
| 运行指示 | PC13 1Hz 心跳 |

## 硬件

STM32F103C8T6 最小系统板、HC-SR04（仿真用 SRF04 模型）、两个 10k 电位器（踏板开度、减速度模拟）、刹车灯 LED、左右转向灯 LED、蜂鸣器、继电器模块、5V 电源。

### 引脚分配

| 引脚 | 功能 |
| --- | --- |
| PA0 | ADC1_IN0 制动踏板电位器（0–3.3V → 0–100%） |
| PA1 | ADC1_IN1 减速度模拟电位器（仿真时代替 MPU6050） |
| PA2 / PA3 | USART2 TX / RX（9600bps） |
| PB0 | HC-SR04 Trig |
| PB1 | HC-SR04 Echo（EXTI1 + TIM2 测回波脉宽） |
| PB5 | 蜂鸣器 |
| PB6 | 刹车灯 PWM（TIM4_CH1） |
| PB7 / PB8 | 左 / 右转向灯（双闪） |
| PC13 | 运行灯（低电平点亮） |
| PC14 | 继电器（模拟制动助力泵） |

## 状态机

| 状态 | 触发条件 | 表现 |
| --- | --- | --- |
| NORMAL | 默认 | 刹车灯随踏板，转向灯熄灭 |
| FOLLOW_WARN | 距离 <150cm | 双黄灯 300ms 闪烁 |
| FOLLOW_URGENT | 距离 <50cm | 双黄灯 100ms 急闪 + 蜂鸣器 |
| BRAKE_ASSIST | 踏板变化率 ≥6 且减速度 ≥50% | 双闪 250ms、蜂鸣器、继电器吸合，持续 2s |

## 快速开始

### 编译

需要 `arm-none-eabi-gcc`（Ubuntu：`sudo apt install gcc-arm-none-eabi`；Windows 可用 Arm GNU Toolchain）：

```bash
cd firmware
make          # 实物固件：72MHz + 58us/cm  → fw.hex
make sim      # 仿真固件：8MHz + 66us/cm 标定 → fw_sim.hex
```

仓库的 `firmware/prebuilt/` 里带了两个预编译版本：

| 文件 | 用途 |
| --- | --- |
| `fw_72mhz.hex` | 实物板（HSE 8MHz × PLL9） |
| `fw_sim_8mhz.hex` | Proteus 仿真（8MHz 直跑 + SIM_CAL） |

### Proteus 仿真

1. 新建工程，MCU 选 **STM32F103C8**，**OSC Frequency = 8MHz，Clock Scale = off**。Proteus 的 BLUEPILL 模型不读固件里的 PLL 配置，核心时钟 = OSC × Clock Scale，这一步错了串口就是乱码；
2. Program File 选 `firmware/prebuilt/fw_sim_8mhz.hex`；
3. 按 `docs/阶段①_接线_修订版.md` → `docs/阶段②_刹车辅助与灯光预警.md` → `docs/阶段③_超声波跟车预警.md` 接线，接虚拟终端（9600, 8N1）看数据。

串口输出示例（10Hz）：

```
P=42 R=3 D=61 V=118 W=7854 S=1
```

字段：踏板开度 %、踏板变化率、减速度 %、距离 cm、回波脉宽 us、状态机编号。

## 实验数据

超声波测距标定（详见 [docs/测距标定实验.md](docs/测距标定实验.md)）：

- Proteus SRF04 模型实测回波系数 **66.4 us/cm**，不是手册的 58 us/cm，所以仿真固件用 `-DSIM_CAL` 按 66 us/cm 换算；
- 20–200cm 七个测点，标定后最大相对误差 **2.5%**，绝对误差 ≤1.1cm；
- 原始数据：[data/距离标定实验数据.xlsx](data/距离标定实验数据.xlsx)。

## 仓库结构

```
firmware/          固件源码（裸机，直接操作寄存器）
  prebuilt/        预编译 hex
docs/              接线、Proteus、实验记录
data/              测距标定原始数据
figures/           运行截图（Proteus）
```

## 文档索引

- 仿真：[Proteus仿真搭建手册.md](docs/Proteus仿真搭建手册.md)
- 接线：[阶段①_接线_修订版.md](docs/阶段①_接线_修订版.md) · [阶段②_刹车辅助与灯光预警.md](docs/阶段②_刹车辅助与灯光预警.md) · [阶段③_超声波跟车预警.md](docs/阶段③_超声波跟车预警.md)
- 实验：[测距标定实验.md](docs/测距标定实验.md) · [数据采集清单.md](docs/数据采集清单.md)
- 配图：[figures/](figures/) · [配图截图清单.md](docs/配图截图清单.md)

## 说明

- 固件是裸机 + 直接写寄存器，不依赖 HAL / CubeMX，`make` 可复现；
- 仿真与实物的差异主要在声速系数：仿真 66 us/cm、实物 58 us/cm，用 `SIM_CAL` 切换；
- Proteus 的 STM32 模型对 PLL 支持不完整，所以仿真用 8MHz 直跑版本；
- 紧急制动判定同时看踏板变化率和减速度，避免单阈值误触发。

## 许可证

[MIT](LICENSE) © 2026 yya12123
