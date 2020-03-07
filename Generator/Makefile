#
#Executable name
#
EXE_NAME=generator

#
#Folder to place compiled objects
#
BUILDDIR = build

#
#Verbose mode
#
VERBOSE=no

#
#Colorize ouput
#
COLORIZE=no

#
#Enable binary creation
#
MAKE_BINARY=yes

#
#Enable executable creation
#
MAKE_EXECUTABLE=yes

#
#Enable shared library creation
#
MAKE_SHARED_LIB=no

#
#Enable static library creation
#
MAKE_STATIC_LIB=no

#
#Enable MAP-file creation
#
CREATE_MAP=yes

#
#Tool-chain prefix
#
TCHAIN = arm-none-eabi-

#
#CPU specific options
#
MCPU += -mcpu=cortex-m4
MCPU += -mthumb
MCPU += -mfpu=fpv4-sp-d16
MCPU += -mfloat-abi=hard


#
#C language dialect
#
CDIALECT = gnu99

#
#C++ language dialect
#
CXXDIALECT = c++0x

#
#Optimization
#
OPT_LVL = 1

#
#Debug information options
#
DBG_OPTS = -gdwarf-2 -ggdb -g

#
#Additional C flags
#
CFLAGS   += -fdata-sections -ffunction-sections 
CFLAGS   += -fsingle-precision-constant

CFLAGS   += -Wno-pointer-sign
CFLAGS   += -fmessage-length=0
CFLAGS   += -fno-exceptions
CFLAGS   += -fno-move-loop-invariants -ffreestanding

# CFLAGS   += -Wall -Wextra -Wstrict-prototypes  -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs
# CFLAGS   += -Wshadow -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wno-unused
# CFLAGS   += -pedantic -Wcast-qual -Wcast-align

# CFLAGS   += -Werror
# CFLAGS   += -Wundef 
# CFLAGS   += -Wmissing-declarations 
# CFLAGS   += -Wredundant-decls

#
#Additional CPP flags
#
CXXFLAGS += -fdata-sections -ffunction-sections 
CXXFLAGS += -fsingle-precision-constant

#
#Additional linker flags
#
# LDFLAGS  += -nostartfiles
LDFLAGS  += -specs=nano.specs
LDFLAGS  += -Wl,--gc-sections
LDFLAGS  += -u _printf_float
LDFLAGS  += -g
# LDFLAGS  += -u _scanf_float
LDFLAGS  += -Wl,--print-memory-usage

#
#Additional link objects
#
#EXT_OBJECTS +=

#
#Additional static libraries
#
# EXT_LIBS +=c
# EXT_LIBS +=nosys 
EXT_LIBS +=gcc
EXT_LIBS +=c
EXT_LIBS +=nosys
EXT_LIBS +=STemWin526_CM4_GCC
EXT_LIBS +=m
LDINCLUDE +=src/STemWIN

#
#Preprocessor definitions
#
PPDEFS  += USE_STDPERIPH_DRIVER 
PPDEFS  += STM32F4xx
PPDEFS  += STM32F407VG
PPDEFS  += __ASSEMBLY__
PPDEFS  += __FPU_USED
PPDEFS  += SUPPORT_CPLUSPLUS

#
#Include directories
#
INCDIR  += .
INCDIR  += cmsis
INCDIR  += cmsis_boot
INCDIR  += cmsis_lib/include
INCDIR  += FreeRTOS/include
INCDIR  += FreeRTOS/Source
INCDIR  += FreeRTOS/Source/include
INCDIR  += FreeRTOS
INCDIR  += src/FAST_ILI9328
INCDIR  += src/PCD8544
INCDIR  += src/NRF24L01
INCDIR  += src/STemWIN
INCDIR  += src/STemWIN/inc
INCDIR  += src/XPT2046
INCDIR  += semihosting
INCDIR  += Demo
INCDIR  += ../lib
INCDIR  += ../lib/rfm12b
INCDIR  += base/Inc 
INCDIR  += base/Drivers/STM32F4xx_HAL_Driver/Inc 
INCDIR  += base/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy 
INCDIR  += base/Drivers/CMSIS/Device/ST/stm32f4xx/Include 
INCDIR  += base/Drivers/CMSIS/Include

# INCDIR  += app/common/canopennode
# INCDIR  += app/common/canopennode/stack


#
#Source files
#
SOURCES += cmsis_boot/startup/startup_stm32f4xx.c
SOURCES += cmsis_boot/system_stm32f4xx.c
SOURCES += $(wildcard cmsis_lib/source/*.c)
SOURCES += $(wildcard syscalls/*.c)

# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_crc.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c 
# # SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_eth.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rng.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c 
# SOURCES += base/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c 

SOURCES += $(wildcard *.c)
SOURCES += $(wildcard Demo/*.c)
SOURCES += $(wildcard src/*.c)
SOURCES += $(wildcard FreeRTOS/Source/*.c)
SOURCES += $(wildcard src/FAST_ILI9328/*.c)
# SOURCES += $(wildcard src/PCD8544/*.c)
SOURCES += $(wildcard src/XPT2046/*.c)

LLIBDIR   = $(patsubst %,-L%,$(LDINCLUDE))

#
#Linker scripts
#
LDSCRIPT += STM32F407VGTx_FLASH.ld

include core.mk

.PHONY : tags
tags:
	ctags -R $(SOURCES) $(INCDIR) 