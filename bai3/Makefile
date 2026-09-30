PROJECT = bai3

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CPU = cortex-m3

CFLAGS = -mcpu=$(CPU) \
         -mthumb \
         -mfloat-abi=soft \
         -O0 \
         -g3 \
         -Wall \
         -ffunction-sections \
         -fdata-sections \
         -DSTM32F10X_MD \
         -DUSE_STDPERIPH_DRIVER

INC = -Iinc \
      -I$(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/inc \
      -I$(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/CMSIS/CM3/CoreSupport \
      -I$(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x

LDSCRIPT = stm32f103c8.ld

LDFLAGS = -mcpu=$(CPU) \
          -mthumb \
          -mfloat-abi=soft \
          -T$(LDSCRIPT) \
          -Wl,--gc-sections \
          -Wl,-Map=build/$(PROJECT).map \
          --specs=nano.specs \
          --specs=nosys.specs

SRC = src/main.c \
      $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c \
      $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/misc.c \
      $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c \
      $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.c \
      $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c \
      $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c \
      $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c \
      $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.c \
      startup_stm32f10x_md.s

OBJ = $(patsubst %.c,build/%.o,$(notdir $(SRC)))

vpath %.c src
vpath %.c $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
vpath %.c $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0/Libraries/STM32F10x_StdPeriph_Driver/src

all: build/$(PROJECT).elf build/$(PROJECT).bin
	@echo ""
	@echo "===== BUILD SUCCESS ====="
	$(SIZE) build/$(PROJECT).elf

build/$(PROJECT).elf: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDFLAGS) -o $@

build/$(PROJECT).bin: build/$(PROJECT).elf
	$(OBJCOPY) -O binary $< $@

build/%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -rf build/*.o
	rm -rf build/*.elf
	rm -rf build/*.bin
	rm -rf build/*.map
