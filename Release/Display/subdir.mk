################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Display/display.c \
../Display/display_offsets.c \
../Display/fonts.c \
../Display/ili9341.c \
../Display/st7789.c 

S_UPPER_SRCS += \
../Display/display_asm.S 

OBJS += \
./Display/display.o \
./Display/display_asm.o \
./Display/display_offsets.o \
./Display/fonts.o \
./Display/ili9341.o \
./Display/st7789.o 

S_UPPER_DEPS += \
./Display/display_asm.d 

C_DEPS += \
./Display/display.d \
./Display/display_offsets.d \
./Display/fonts.d \
./Display/ili9341.d \
./Display/st7789.d 


# Each subdirectory must supply rules for building sources it contributes
Display/%.o: ../Display/%.c Display/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Ofast -fmessage-length=0 -ffunction-sections -fdata-sections  -g -DAT_START_F403A_V1 -DNDEBUG -DAT32F403ACGU7 -DUSE_STDPERIPH_DRIVER -I"../include" -I"F:\AT32-workspace\AT32F403ACGU7_display_spi_dma\Display" -I"../include/libraries/drivers/inc" -I"../include/libraries/cmsis/cm4/core_support" -I"../include/libraries/cmsis/cm4/device_support" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Display/%.o: ../Display/%.S Display/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Ofast -fmessage-length=0 -ffunction-sections -fdata-sections  -g -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


