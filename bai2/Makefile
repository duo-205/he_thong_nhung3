PROJECT = bai2

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CPU = cortex-m3

CFLAGS = -mcpu=$(CPU) -mthumb
CFLAGS += -mfloat-abi=soft
CFLAGS += -O2
CFLAGS += -Wall
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections

CFLAGS += -DSTM32F10X_MD
CFLAGS += -DUSE_STDPERIPH_DRIVER

INC = \
	-Iinc \
	-I../bai1/inc \
	-I../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/inc \
	-I../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/CMSIS/CM3/CoreSupport \
	-I../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x

SRC = \
	src/main.c \
	src/max7219.c \
	../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/misc.c \
	../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c \
	../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c \
	../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c \
	../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c

STARTUP = startup_stm32f10x_md.s
LDSCRIPT = stm32f103c8.ld

CFLAGS += -Iinc
LDFLAGS = -T$(LDSCRIPT)
LDFLAGS += -mcpu=$(CPU) -mthumb
LDFLAGS += -mfloat-abi=soft
LDFLAGS += --specs=nano.specs
LDFLAGS += --specs=nosys.specs
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=build/$(PROJECT).map

OBJ = \
	build/main.o \
	build/max7219.o \
	build/misc.o \
	build/stm32f10x_gpio.o \
	build/stm32f10x_rcc.o \
	build/stm32f10x_spi.o \
	build/system_stm32f10x.o

all: build/$(PROJECT).bin

build/$(PROJECT).elf: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(STARTUP) $(LDFLAGS) -o $@
	$(SIZE) $@

build/$(PROJECT).bin: build/$(PROJECT).elf
	$(OBJCOPY) -O binary $< $@
	@echo ""
	@echo "===== BUILD SUCCESS ====="
	@ls -lh $@

build/main.o: src/main.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

build/max7219.o: src/max7219.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

build/misc.o: ../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/misc.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

build/stm32f10x_gpio.o: ../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

build/stm32f10x_rcc.o: ../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

build/stm32f10x_spi.o: ../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

build/system_stm32f10x.o: ../../Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f build/*.o
	rm -f build/*.elf
	rm -f build/*.bin
	rm -f build/*.map

.PHONY: all clean
