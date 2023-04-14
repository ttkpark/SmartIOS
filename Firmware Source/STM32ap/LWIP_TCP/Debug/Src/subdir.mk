################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/app_ethernet.c \
../Src/enc28j60.c \
../Src/ethernetif.c \
../Src/main.c \
../Src/stm32f4xx_hal_msp.c \
../Src/stm32f4xx_it.c \
../Src/system_stm32f4xx.c \
../Src/tcp_echoserver.c 

OBJS += \
./Src/app_ethernet.o \
./Src/enc28j60.o \
./Src/ethernetif.o \
./Src/main.o \
./Src/stm32f4xx_hal_msp.o \
./Src/stm32f4xx_it.o \
./Src/system_stm32f4xx.o \
./Src/tcp_echoserver.o 

C_DEPS += \
./Src/app_ethernet.d \
./Src/enc28j60.d \
./Src/ethernetif.d \
./Src/main.d \
./Src/stm32f4xx_hal_msp.d \
./Src/stm32f4xx_it.d \
./Src/system_stm32f4xx.d \
./Src/tcp_echoserver.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Inc" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/STM32F4xx_HAL_Driver/Inc" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/CMSIS/Include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/system" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/lwip" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/ipv4" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/netif" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/posix" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/posix/sys" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/system/arch"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


