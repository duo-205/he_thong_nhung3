PROJECT = bai4

BUILD_DIR = build
SRC_DIR = src
INC_DIR = inc

LIB_DIR = $(HOME)/STM32/Libraries/STM32F10x_StdPeriph_Lib_V3.6.0

DRIVER_DIR = $(LIB_DIR)/Libraries/STM32F10x_StdPeriph_Driver
CMSIS_DIR = $(LIB_DIR)/Libraries/CMSIS
DEVICE_DIR = $(CMSIS_DIR)/CM3/DeviceSupport/ST/STM32F10x

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CPU = -mcpu=cortex-m3
THUMB = -mthumb

CFLAGS = $(CPU) $(THUMB)

CFLAGS += -Wall
CFLAGS += -O0
CFLAGS += -g3

CFLAGS += -DSTM32F10X_MD
CFLAGS += -DUSE_STDPERIPH_DRIVER

CFLAGS += -I$(INC_DIR)
CFLAGS += -I$(DRIVER_DIR)/inc
CFLAGS += -I$(CMSIS_DIR)/CM3/CoreSupport
CFLAGS += -I$(DEVICE_DIR)

LDFLAGS = $(CPU) $(THUMB)
LDFLAGS += -Tstm32f103c8.ld
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map
LDFLAGS += --specs=nano.specs
LDFLAGS += --specs=nosys.specs

SRC = \
	$(SRC_DIR)/main.c \
	$(DEVICE_DIR)/system_stm32f10x.c \
	$(DRIVER_DIR)/src/misc.c \
	$(DRIVER_DIR)/src/stm32f10x_rcc.c \
	$(DRIVER_DIR)/src/stm32f10x_flash.c \
	$(DRIVER_DIR)/src/stm32f10x_gpio.c \
	$(DRIVER_DIR)/src/stm32f10x_adc.c \
	$(DRIVER_DIR)/src/stm32f10x_dma.c \
	$(DRIVER_DIR)/src/stm32f10x_tim.c \
	$(DRIVER_DIR)/src/stm32f10x_usart.c

OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)

STARTUP = startup_stm32f10x_md.s
STARTUP_OBJ = $(BUILD_DIR)/startup_stm32f10x_md.o

all: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJCOPY) -O binary \
	$(BUILD_DIR)/$(PROJECT).elf \
	$(BUILD_DIR)/$(PROJECT).bin

	@echo ""
	@echo "===== BUILD SUCCESS ====="
	$(SIZE) $(BUILD_DIR)/$(PROJECT).elf

$(BUILD_DIR)/$(PROJECT).elf: $(OBJ) $(STARTUP_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/startup_stm32f10x_md.o: $(STARTUP)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
