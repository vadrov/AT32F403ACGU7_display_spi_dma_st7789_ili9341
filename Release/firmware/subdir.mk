################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../firmware/at32f403a_407_acc.c \
../firmware/at32f403a_407_adc.c \
../firmware/at32f403a_407_bpr.c \
../firmware/at32f403a_407_can.c \
../firmware/at32f403a_407_crc.c \
../firmware/at32f403a_407_crm.c \
../firmware/at32f403a_407_dac.c \
../firmware/at32f403a_407_debug.c \
../firmware/at32f403a_407_dma.c \
../firmware/at32f403a_407_emac.c \
../firmware/at32f403a_407_exint.c \
../firmware/at32f403a_407_flash.c \
../firmware/at32f403a_407_gpio.c \
../firmware/at32f403a_407_i2c.c \
../firmware/at32f403a_407_misc.c \
../firmware/at32f403a_407_pwc.c \
../firmware/at32f403a_407_rtc.c \
../firmware/at32f403a_407_sdio.c \
../firmware/at32f403a_407_spi.c \
../firmware/at32f403a_407_tmr.c \
../firmware/at32f403a_407_usart.c \
../firmware/at32f403a_407_usb.c \
../firmware/at32f403a_407_wdt.c \
../firmware/at32f403a_407_wwdt.c \
../firmware/at32f403a_407_xmc.c 

OBJS += \
./firmware/at32f403a_407_acc.o \
./firmware/at32f403a_407_adc.o \
./firmware/at32f403a_407_bpr.o \
./firmware/at32f403a_407_can.o \
./firmware/at32f403a_407_crc.o \
./firmware/at32f403a_407_crm.o \
./firmware/at32f403a_407_dac.o \
./firmware/at32f403a_407_debug.o \
./firmware/at32f403a_407_dma.o \
./firmware/at32f403a_407_emac.o \
./firmware/at32f403a_407_exint.o \
./firmware/at32f403a_407_flash.o \
./firmware/at32f403a_407_gpio.o \
./firmware/at32f403a_407_i2c.o \
./firmware/at32f403a_407_misc.o \
./firmware/at32f403a_407_pwc.o \
./firmware/at32f403a_407_rtc.o \
./firmware/at32f403a_407_sdio.o \
./firmware/at32f403a_407_spi.o \
./firmware/at32f403a_407_tmr.o \
./firmware/at32f403a_407_usart.o \
./firmware/at32f403a_407_usb.o \
./firmware/at32f403a_407_wdt.o \
./firmware/at32f403a_407_wwdt.o \
./firmware/at32f403a_407_xmc.o 

C_DEPS += \
./firmware/at32f403a_407_acc.d \
./firmware/at32f403a_407_adc.d \
./firmware/at32f403a_407_bpr.d \
./firmware/at32f403a_407_can.d \
./firmware/at32f403a_407_crc.d \
./firmware/at32f403a_407_crm.d \
./firmware/at32f403a_407_dac.d \
./firmware/at32f403a_407_debug.d \
./firmware/at32f403a_407_dma.d \
./firmware/at32f403a_407_emac.d \
./firmware/at32f403a_407_exint.d \
./firmware/at32f403a_407_flash.d \
./firmware/at32f403a_407_gpio.d \
./firmware/at32f403a_407_i2c.d \
./firmware/at32f403a_407_misc.d \
./firmware/at32f403a_407_pwc.d \
./firmware/at32f403a_407_rtc.d \
./firmware/at32f403a_407_sdio.d \
./firmware/at32f403a_407_spi.d \
./firmware/at32f403a_407_tmr.d \
./firmware/at32f403a_407_usart.d \
./firmware/at32f403a_407_usb.d \
./firmware/at32f403a_407_wdt.d \
./firmware/at32f403a_407_wwdt.d \
./firmware/at32f403a_407_xmc.d 


# Each subdirectory must supply rules for building sources it contributes
firmware/%.o: ../firmware/%.c firmware/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Ofast -fmessage-length=0 -ffunction-sections -fdata-sections  -g -DAT_START_F403A_V1 -DNDEBUG -DAT32F403ACGU7 -DUSE_STDPERIPH_DRIVER -I"../include" -I"F:\AT32-workspace\AT32F403ACGU7_display_spi_dma\Display" -I"../include/libraries/drivers/inc" -I"../include/libraries/cmsis/cm4/core_support" -I"../include/libraries/cmsis/cm4/device_support" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


