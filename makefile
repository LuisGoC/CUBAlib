#Nombre del proyecto
TARGET = temp
#Archivos a compilar
SRCS  = main.c app_ints.c app_msps.c startup_stm32g0b1xx.s system_stm32g0xx.c 
SRCS += stm32g0xx_hal.c stm32g0xx_hal_cortex.c stm32g0xx_hal_rcc.c stm32g0xx_hal_flash.c
SRCS += stm32g0xx_hal_gpio.c stm32g0xx_hal_fdcan.c stm32g0xx_hal_uart.c stm32g0xx_hal_uart_ex.c
SRCS += stm32g0xx_hal_dma.c CUBA.c
#archivo linker a usar
LINKER = linker.ld
#Simbolos gloobales del programa (#defines globales)
SYMBOLS = -DSTM32G0B1xx -DUSE_HAL_DRIVER
#directorios con archivos a compilar (.c y .s)
SRC_PATHS  = app
SRC_PATHS += cmsisg0/startups
SRC_PATHS += halg0/Src
#direcotrios con archivos .h
INC_PATHS  = app
INC_PATHS += cmsisg0/core
INC_PATHS += cmsisg0/registers
INC_PATHS += halg0/Inc

#compilador y opciones de compilacion
TOOLCHAIN = arm-none-eabi
CPU = -mcpu=cortex-m0 -mthumb -mfloat-abi=soft
CFLAGS  = $(CPU) -Wall -g3 -O0 -std=c99
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -MMD -MP
AFLAGS = $(CPU)
LFLAGS = $(CPU) -Wl,--gc-sections --specs=rdimon.specs --specs=nano.specs -Wl,-Map=Build/$(TARGET).map

#substituccion de prefijos y postfijos 
OBJS = $(SRCS:%.c=Build/obj/%.o)
OBJS := $(OBJS:%.s=Build/obj/%.o)

DEPS = $(OBJS:%.o=%.d)
VPATH = $(SRC_PATHS)
INCLS = $(addprefix -I ,$(INC_PATHS))

#Instrucciones de compilacion
all : build $(TARGET)

$(TARGET) : $(addprefix Build/, $(TARGET).elf)
	$(TOOLCHAIN)-objcopy -Oihex $< Build/$(TARGET).hex
	$(TOOLCHAIN)-objdump -S $< > Build/$(TARGET).lst
	$(TOOLCHAIN)-size --format=berkeley $<

Build/$(TARGET).elf : $(OBJS)
	$(TOOLCHAIN)-gcc $(LFLAGS) -T $(LINKER) -o $@ $^

Build/obj/%.o : %.c
	$(TOOLCHAIN)-gcc $(CFLAGS) $(INCLS) $(SYMBOLS) -o $@ -c $<

Build/obj/%.o : %.s
	$(TOOLCHAIN)-as $(AFLAGS) -o $@ -c $<

build :
	mkdir -p Build/obj

-include $(DEPS)

#borrar archivos generados
clean :
	rm -rf Build/

#---flash the image into the mcu-------------------------------------------------------------------
flash :
	openocd -f interface/stlink.cfg -f target/stm32g0x.cfg -c "program Build/$(TARGET).hex verify reset" -c shutdown

#---open a debug server conection------------------------------------------------------------------
open :
#	openocd -f interface/stlink.cfg -f target/stm32g0x.cfg -c "reset_config srst_only srst_nogate"
	JLinkGDBServer -if SWD -device stm32g0b1re -nogui

#---launch a debug session, NOTE: is mandatory to previously open a debug server session-----------
debug :
	arm-none-eabi-gdb Build/$(TARGET).elf -iex "set auto-load safe-path /"
