################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/lwIP/src/core/def.c \
../Middlewares/Third_Party/lwIP/src/core/dhcp.c \
../Middlewares/Third_Party/lwIP/src/core/dns.c \
../Middlewares/Third_Party/lwIP/src/core/init.c \
../Middlewares/Third_Party/lwIP/src/core/mem.c \
../Middlewares/Third_Party/lwIP/src/core/memp.c \
../Middlewares/Third_Party/lwIP/src/core/netif.c \
../Middlewares/Third_Party/lwIP/src/core/pbuf.c \
../Middlewares/Third_Party/lwIP/src/core/raw.c \
../Middlewares/Third_Party/lwIP/src/core/stats.c \
../Middlewares/Third_Party/lwIP/src/core/sys.c \
../Middlewares/Third_Party/lwIP/src/core/tcp.c \
../Middlewares/Third_Party/lwIP/src/core/tcp_in.c \
../Middlewares/Third_Party/lwIP/src/core/tcp_out.c \
../Middlewares/Third_Party/lwIP/src/core/timers.c \
../Middlewares/Third_Party/lwIP/src/core/udp.c 

OBJS += \
./Middlewares/Third_Party/lwIP/src/core/def.o \
./Middlewares/Third_Party/lwIP/src/core/dhcp.o \
./Middlewares/Third_Party/lwIP/src/core/dns.o \
./Middlewares/Third_Party/lwIP/src/core/init.o \
./Middlewares/Third_Party/lwIP/src/core/mem.o \
./Middlewares/Third_Party/lwIP/src/core/memp.o \
./Middlewares/Third_Party/lwIP/src/core/netif.o \
./Middlewares/Third_Party/lwIP/src/core/pbuf.o \
./Middlewares/Third_Party/lwIP/src/core/raw.o \
./Middlewares/Third_Party/lwIP/src/core/stats.o \
./Middlewares/Third_Party/lwIP/src/core/sys.o \
./Middlewares/Third_Party/lwIP/src/core/tcp.o \
./Middlewares/Third_Party/lwIP/src/core/tcp_in.o \
./Middlewares/Third_Party/lwIP/src/core/tcp_out.o \
./Middlewares/Third_Party/lwIP/src/core/timers.o \
./Middlewares/Third_Party/lwIP/src/core/udp.o 

C_DEPS += \
./Middlewares/Third_Party/lwIP/src/core/def.d \
./Middlewares/Third_Party/lwIP/src/core/dhcp.d \
./Middlewares/Third_Party/lwIP/src/core/dns.d \
./Middlewares/Third_Party/lwIP/src/core/init.d \
./Middlewares/Third_Party/lwIP/src/core/mem.d \
./Middlewares/Third_Party/lwIP/src/core/memp.d \
./Middlewares/Third_Party/lwIP/src/core/netif.d \
./Middlewares/Third_Party/lwIP/src/core/pbuf.d \
./Middlewares/Third_Party/lwIP/src/core/raw.d \
./Middlewares/Third_Party/lwIP/src/core/stats.d \
./Middlewares/Third_Party/lwIP/src/core/sys.d \
./Middlewares/Third_Party/lwIP/src/core/tcp.d \
./Middlewares/Third_Party/lwIP/src/core/tcp_in.d \
./Middlewares/Third_Party/lwIP/src/core/tcp_out.d \
./Middlewares/Third_Party/lwIP/src/core/timers.d \
./Middlewares/Third_Party/lwIP/src/core/udp.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/lwIP/src/core/%.o: ../Middlewares/Third_Party/lwIP/src/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Inc" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/STM32F4xx_HAL_Driver/Inc" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/CMSIS/Include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/system" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/lwip" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/ipv4" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/netif" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/posix" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/posix/sys" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/system/arch"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


