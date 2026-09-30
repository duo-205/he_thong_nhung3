PROJECT = bai1

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CPU = -mcpu=cortex-m3 -mthumb

LIB = $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0

CFLAGS = $(CPU) \
         -Wall \
         -O2 \
         -ffunction-sections \
         -fdata-sections \
         -DSTM32F10X_MD \
         -DUSE_STDPERIPH_DRIVER \
         -Iinc \
         -I$(LIB)/Libraries/STM32F10x_StdPeriph_Driver/inc \
         -I$(LIB)/Libraries/CMSIS/CM3/CoreSupport \
         -I$(LIB)/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x

LDFLAGS = $(CPU) \
          -Tstm32f103c8.ld \
          -Wl,--gc-sections \
          -Wl,-Map=build/$(PROJECT).map \
          --specs=nano.specs \
          --specs=nosys.specs

SRC = \
    src/main.c \
    src/i2c.c \
    src/ssd1306.c \
    $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c \
    $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c \
    $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.c \
    $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/misc.c \
    $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.c \
    $(LIB)/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c

OBJ = $(SRC:.c=.o)

STARTUP = startup_stm32f10x_md.o


all: build/$(PROJECT).bin


build/$(PROJECT).elf: $(OBJ) $(STARTUP)
	$(CC) $(OBJ) $(STARTUP) $(LDFLAGS) -o $@
	$(SIZE) $@


build/$(PROJECT).bin: build/$(PROJECT).elf
	$(OBJCOPY) -O binary $< $@
	@echo ""
	@echo "===== BUILD SUCCESS ====="
	@ls -lh $@


src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@


$(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/%.o: \
$(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@


$(LIB)/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/%.o: \
$(LIB)/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/%.c
	$(CC) $(CFLAGS) -c $< -o $@


startup_stm32f10x_md.o: startup_stm32f10x_md.s
	$(CC) $(CPU) \
	    -DSTM32F10X_MD \
	    -c $< -o $@


clean:
	rm -f src/*.o
	rm -f $(LIB)/Libraries/STM32F10x_StdPeriph_Driver/src/*.o
	rm -f $(LIB)/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/*.o
	rm -f startup_stm32f10x_md.o
	rm -f build/$(PROJECT).elf
	rm -f build/$(PROJECT).bin
	rm -f build/$(PROJECT).map


.PHONY: all clean
