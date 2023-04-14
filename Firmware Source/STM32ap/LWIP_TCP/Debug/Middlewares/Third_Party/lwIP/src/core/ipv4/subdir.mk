################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/lwIP/src/core/ipv4/autoip.c \
../Middlewares/Third_Party/lwIP/src/core/ipv4/icmp.c \
../Middlewares/Third_Party/lwIP/src/core/ipv4/igmp.c \
../Middlewares/Third_Party/lwIP/src/core/ipv4/inet.c \
../Middlewares/Third_Party/lwIP/src/core/ipv4/inet_chksum.c \
../Middlewares/Third_Party/lwIP/src/core/ipv4/ip.c \
../Middlewares/Third_Party/lwIP/src/core/ipv4/ip_addr.c \
../Middlewares/Third_Party/lwIP/src/core/ipv4/ip_frag.c 

OBJS += \
./Middlewares/Third_Party/lwIP/src/core/ipv4/autoip.o \
./Middlewares/Third_Party/lwIP/src/core/ipv4/icmp.o \
./Middlewares/Third_Party/lwIP/src/core/ipv4/igmp.o \
./Middlewares/Third_Party/lwIP/src/core/ipv4/inet.o \
./Middlewares/Third_Party/lwIP/src/core/ipv4/inet_chksum.o \
./Middlewares/Third_Party/lwIP/src/core/ipv4/ip.o \
./Middlewares/Third_Party/lwIP/src/core/ipv4/ip_addr.o \
./Middlewares/Third_Party/lwIP/src/core/ipv4/ip_frag.o 

C_DEPS += \
./Middlewares/Third_Party/lwIP/src/core/ipv4/autoip.d \
./Middlewares/Third_Party/lwIP/src/core/ipv4/icmp.d \
./Middlewares/Third_Party/lwIP/src/core/ipv4/igmp.d \
./Middlewares/Third_Party/lwIP/src/core/ipv4/inet.d \
./Middlewares/Third_Party/lwIP/src/core/ipv4/inet_chksum.d \
./Middlewares/Third_Party/lwIP/src/core/ipv4/ip.d \
./Middlewares/Third_Party/lwIP/src/core/ipv4/ip_addr.d \
./Middlewares/Third_Party/lwIP/src/core/ipv4/ip_frag.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/lwIP/src/core/ipv4/%.o: ../Middlewares/Third_Party/lwIP/src/core/ipv4/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Inc" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/STM32F4xx_HAL_Driver/Inc" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/CMSIS/Include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/system" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/lwip" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/ipv4" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/netif" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/posix" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/posix/sys" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/system/arch"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


