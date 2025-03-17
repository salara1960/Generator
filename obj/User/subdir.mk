################################################################################
# MRS Version: 2.1.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/AD9833.c \
../User/ch32x035_it.c \
../User/enc.c \
../User/main.c \
../User/ssd1306.c \
../User/system_ch32x035.c 

C_DEPS += \
./User/AD9833.d \
./User/ch32x035_it.d \
./User/enc.d \
./User/main.d \
./User/ssd1306.d \
./User/system_ch32x035.d 

OBJS += \
./User/AD9833.o \
./User/ch32x035_it.o \
./User/enc.o \
./User/main.o \
./User/ssd1306.o \
./User/system_ch32x035.o 



# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/alarm/mrs_community-workspace/CH32X035/Generator/Debug" -I"/home/alarm/mrs_community-workspace/CH32X035/Generator/Core" -I"/home/alarm/mrs_community-workspace/CH32X035/Generator/User" -I"/home/alarm/mrs_community-workspace/CH32X035/Generator/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
