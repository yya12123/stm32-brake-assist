# Makefile -- 交叉编译到 STM32F103C8T6
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

CFLAGS  = -mcpu=cortex-m3 -mthumb -O2 -g -Wall -Wextra \
          -ffunction-sections -fdata-sections -std=gnu11 -fno-common
LDFLAGS = -T linker.ld -nostartfiles -Wl,--gc-sections -Wl,-Map=fw.map

SRCS = startup.c main.c
OBJS = $(SRCS:.c=.o)

all: fw.hex size

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

fw.elf: $(OBJS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

fw.hex: fw.elf
	$(OBJCOPY) -O ihex $< $@

size: fw.elf
	$(SIZE) $<

# ---------------------------------------------------------------------------
# 仿真构建：8MHz 直跑 + Proteus 声速标定（66us/cm）
#   对应 docs/Proteus仿真搭建手册.md 里的 fw_demo.hex
# ---------------------------------------------------------------------------
SIM_CFLAGS = $(CFLAGS) -DRUN_8MHZ -DSIM_CAL

sim: fw_sim.hex
	$(SIZE) fw_sim.elf

sim_%.o: %.c
	$(CC) $(SIM_CFLAGS) -c $< -o $@

fw_sim.elf: sim_startup.o sim_main.o linker.ld
	$(CC) $(SIM_CFLAGS) -T linker.ld -nostartfiles -Wl,--gc-sections -Wl,-Map,fw_sim.map sim_startup.o sim_main.o -o $@

fw_sim.hex: fw_sim.elf
	$(OBJCOPY) -O ihex $< $@

clean:
	rm -f *.o fw.elf fw.hex fw.map fw_sim.elf fw_sim.hex fw_sim.map

.PHONY: all sim size clean
