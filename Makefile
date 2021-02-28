######################################
# target
######################################
TARGET = p25rx_display


######################################
# building variables
######################################
# debug build?
DEBUG = 0
# optimization
OPT = -O2


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
usb_host/cdc_receive.c \
usb_host/usbh_conf.c \
usb_host/cdc_configuration.c \
drivers/STM32_USB_Host_Library/Class/CDC/Src/usbh_cdc.c \
drivers/STM32_USB_Host_Library/Core/Src/usbh_ioreq.c \
drivers/STM32_USB_Host_Library/Core/Src/usbh_core.c \
drivers/STM32_USB_Host_Library/Core/Src/usbh_pipes.c \
drivers/STM32_USB_Host_Library/Core/Src/usbh_ctlreq.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.c \
lcd_ili9341/lcd_buttons.c \
lcd_ili9341/lcd.c \
lcd_ili9341/lcd_page1.c \
lcd_ili9341/lcd_page2.c \
lcd_ili9341/lcd_page3.c \
lcd_ili9341/lcd_page4.c \
lcd_ili9341/lcd_page5.c \
lcd_ili9341/fonts.c \
main.c \
syscalls.c \
std_io.c \
stm32h7xx_it.c \
stm32h7xx_hal_msp.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc_ex.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdios.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s.c \
drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c \
system_stm32h7xx.c \
#drivers/CMSIS/DSP/Source/FastMathFunctions/arm_cos_f32.c \
#drivers/CMSIS/DSP/Source/CommonTables/arm_common_tables.c \
#drivers/CMSIS/DSP/Source/SupportFunctions/arm_float_to_q15.c \
#drivers/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_conj_f32.c \
#drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_decimate_fast_q15.c \
#drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_decimate_init_q15.c \
#drivers/CMSIS/DSP/Source/StatisticsFunctions/arm_std_f32.c \
#drivers/CMSIS/DSP/Source/StatisticsFunctions/arm_rms_f32.c \
#drivers/CMSIS/DSP/Source/BasicMathFunctions/arm_cmplx_dot_prod_f32.c \
#drivers/CMSIS/DSP/Source/BasicMathFunctions/arm_dot_prod_f32.c \
#drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix2_init_f32.o \
#drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix2_f32.o \
#drivers/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix8_f32.o \
#drivers/CMSIS/DSP/Source/TransformFunctions/arm_cmplx_mag_f32.o \
#drivers/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal.o \
#drivers/CMSIS/DSP/Source/TransformFunctions/arm_const_structs.o \

# ASM sources
ASM_SOURCES =  \
startup_stm32h743xx.s


#######################################
# binaries
#######################################
GCC_PATH = ./arm-toolchain/bin
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m7

# fpu
FPU = -mfpu=fpv5-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32H743xx


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-I./ \
-Idrivers/STM32H7xx_HAL_Driver/Inc \
-Idrivers/STM32H7xx_HAL_Driver/Inc/Legacy \
-Idrivers/CMSIS/Device/ST/STM32H7xx/Include \
-Idrivers/CMSIS/Include \
-Iusb_host \
-I./drivers/STM32_USB_Host_Library/Core/Inc/ \
-I./drivers/STM32_USB_Host_Library/Class/CDC/Inc/ \
-Idrivers/STM32H7xx_HAL_Driver/Inc/ \
-Ilcd_ili9341 \
-Idrivers/CMSIS/DSP/Include/


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections 

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = stm32h743_link.ld

# libraries
LIBS = -lc -lm

#use this one if no floating point support in printf is required for reduced code size 
LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	

	cp $(BUILD_DIR)/p25rx_display.bin /tmp/main.bin
	cp $(BUILD_DIR)/p25rx_display.bin $(BUILD_DIR)/p25rx_display_DFU.bin 
	dfu-suffix -a $(BUILD_DIR)/p25rx_display_DFU.bin -v 0483 -p df11 >/dev/null
	
$(BUILD_DIR):
	mkdir $@		


#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)
