# STM32 Scale-Model EV · Brake Assist & Light Warning System

**English** · [简体中文](README.md)

> STM32F103C8T6 · Proteus simulation + physical scale-model car

Press the brake pedal and the brake light brightens in stages. When the system detects an emergency-braking intent it automatically flashes the hazard lights, sounds the buzzer and pulls in a relay that stands in for the brake booster. In parallel an ultrasonic sensor keeps watching the distance ahead and raises a following-distance warning or a danger alarm. Everything runs bare-metal on an STM32F103C8T6 with direct register access and no HAL dependency.

## Features

| Module | Behaviour |
| --- | --- |
| Brake light dimming | Pedal <5% off, <40% 25%, <70% 50%, ≥70% 90% (TIM4_CH1 PWM) |
| Emergency brake assist | Pedal rate ≥6%/10ms **and** deceleration ≥50% → 2s assist: 1Hz hazard flash, buzzer, relay on |
| Following-distance warning | <150cm: both yellow lights blink at 300ms; <50cm: 100ms fast flash + buzzer |
| Telemetry | USART2 9600bps, 10Hz line: `P R D V W S` |
| Heartbeat | PC13 toggles at 1Hz |

## Hardware

STM32F103C8T6 minimum system board, HC-SR04 (SRF04 model in simulation), two 10k potentiometers (pedal position, simulated deceleration), brake-light LED, left/right indicator LEDs, buzzer, relay module, 5V supply.

### Pin map

| Pin | Function |
| --- | --- |
| PA0 | ADC1_IN0 brake pedal potentiometer (0–3.3V → 0–100%) |
| PA1 | ADC1_IN1 simulated deceleration (replaces MPU6050 in simulation) |
| PA2 / PA3 | USART2 TX / RX (9600bps) |
| PB0 | HC-SR04 Trig |
| PB1 | HC-SR04 Echo (EXTI1 + TIM2 pulse width) |
| PB5 | Buzzer |
| PB6 | Brake light PWM (TIM4_CH1) |
| PB7 / PB8 | Left / right indicator (hazard) |
| PC13 | Run LED (active low) |
| PC14 | Relay (simulated brake booster) |

## State machine

| State | Trigger | Behaviour |
| --- | --- | --- |
| NORMAL | default | Brake light follows the pedal, indicators off |
| FOLLOW_WARN | distance <150cm | Both indicators blink at 300ms |
| FOLLOW_URGENT | distance <50cm | 100ms fast flash + buzzer |
| BRAKE_ASSIST | pedal rate ≥6 and deceleration ≥50% | 250ms hazard flash, buzzer, relay, 2s |

## Quick start

### Build

Requires `arm-none-eabi-gcc`:

```bash
cd firmware
make          # hardware firmware: 72MHz + 58us/cm  → fw.hex
make sim      # simulation firmware: 8MHz + 66us/cm  → fw_sim.hex
```

Prebuilt images live in `firmware/prebuilt/`:

| File | Purpose |
| --- | --- |
| `fw_72mhz.hex` | Real board (HSE 8MHz × PLL9) |
| `fw_sim_8mhz.hex` | Proteus (8MHz direct run + SIM_CAL) |

### Proteus simulation

1. New project, MCU **STM32F103C8**, **OSC Frequency = 8MHz, Clock Scale = off**. The Proteus BLUEPILL model ignores the PLL setup in firmware; core clock = OSC × Clock Scale. Get this wrong and the UART prints garbage.
2. Program File: `firmware/prebuilt/fw_sim_8mhz.hex`.
3. Wire it up following `docs/阶段①_接线_修订版.md`, `docs/阶段②_刹车辅助与灯光预警.md`, `docs/阶段③_超声波跟车预警.md`, then attach a virtual terminal (9600, 8N1).

Sample line (10Hz):

```
P=42 R=3 D=61 V=118 W=7854 S=1
```

Fields: pedal %, pedal rate, deceleration %, distance cm, echo width us, state id.

## Measurements

Ultrasonic calibration (see [docs/测距标定实验.md](docs/测距标定实验.md)):

- The Proteus SRF04 model measures **66.4 us/cm**, not the 58 us/cm from the datasheet, so the simulation build uses `-DSIM_CAL` with 66 us/cm;
- Over seven points from 20cm to 200cm the calibrated error stays within **2.5%** relative and 1.1cm absolute;
- Raw data: [data/距离标定实验数据.xlsx](data/距离标定实验数据.xlsx).

## Layout

```
firmware/          bare-metal firmware, direct register access
  prebuilt/        prebuilt hex images
docs/              wiring, Proteus, experiment notes
data/              raw calibration data
figures/           screenshots (Proteus)
```

## Notes

- Bare metal, no HAL/CubeMX; `make` reproduces the build.
- Simulation vs. hardware differs mainly in the speed-of-sound factor: 66 us/cm in Proteus, 58 us/cm on real HC-SR04; switch with `SIM_CAL`.
- Proteus' STM32 model does not fully emulate the PLL, hence the 8MHz build for simulation.
- Emergency braking requires both pedal rate and deceleration, which avoids single-threshold false triggers.

## License

[MIT](LICENSE) © 2026 yya12123
