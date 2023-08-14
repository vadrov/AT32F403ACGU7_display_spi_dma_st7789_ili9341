################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../user/at32f403a_407_clock.c \
../user/at32f403a_407_int.c \
../user/main.c 

OBJS += \
./user/at32f403a_407_clock.o \
./user/at32f403a_407_int.o \
./user/main.o 

C_DEPS += \
./user/at32f403a_407_clock.d \
./user/at32f403a_407_int.d \
./user/main.d 


# Each subdirectory must supply rules for building sources it contributes
user/%.o: ../user/%.c user/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Ofast -fmessage-length=0 -ffunction-sections -fdata-sections  -g -DAT_START_F403A_V1 -DNDEBUG -DAT32F403ACGU7 -DUSE_STDPERIPH_DRIVER -I"../include" -I"F:\AT32-workspace\AT32F403ACGU7_display_spi_dma\Display" -I"../include/libraries/drivers/inc" -I"../include/libraries/cmsis/cm4/core_support" -I"../include/libraries/cmsis/cm4/device_support" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


