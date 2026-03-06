# SPLD_Test firmware - STM32F030K6, GCC (arm-none-eabi)
TARGET = firmware
BUILD_DIR = build

# MCU
MCU = -mcpu=cortex-m0 -mthumb

# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
GDB = $(PREFIX)gdb

# Defines
DEFS = -DSTM32F030x6 -DUSE_HAL_DRIVER

# Include paths
C_INCLUDES = \
-I Core/Inc \
-I Drivers/STM32F0xx_HAL_Driver/Inc \
-I Drivers/STM32F0xx_HAL_Driver/Inc/Legacy \
-I Drivers/CMSIS/Device/ST/STM32F0xx/Include \
-I Drivers/CMSIS/Include \
-ICore/Inc \
-IDrivers/STM32F0xx_HAL_Driver/Inc \
-IDrivers/STM32F0xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32F0xx/Include \
-IDrivers/CMSIS/Include

# Compiler flags
CFLAGS = $(MCU) $(DEFS) $(C_INCLUDES) -Og -Wall -fdata-sections -ffunction-sections
CFLAGS += -g -gdwarf-2

# Linker flags
LDFLAGS = $(MCU) -specs=nano.specs -TSTM32F030K6Tx_FLASH.ld -Wl,-Map=$(BUILD_DIR)/$(TARGET).map
LDFLAGS += -Wl,--gc-sections

# Source files
C_SOURCES = \
Core/Src/main.c \
Core/Src/oled_sh1106.c \
Core/Src/stm32f0xx_it.c \
Core/Src/stm32f0xx_hal_msp.c \
Core/Src/system_stm32f0xx.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dma.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr_ex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash_ex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_exti.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c_ex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_spi.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_spi_ex.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim.c \
Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim_ex.c \
Core/Src/sysmem.c \
Core/Src/syscalls.c

ASM_SOURCES = Core/Startup/startup_stm32f030x6.s

# Objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# Default target
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(CP) -O binary $< $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(CP) -O ihex $< $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
