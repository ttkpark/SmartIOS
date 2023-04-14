################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/lwIP/src/api/api_lib.c \
../Middlewares/Third_Party/lwIP/src/api/api_msg.c \
../Middlewares/Third_Party/lwIP/src/api/err.c \
../Middlewares/Third_Party/lwIP/src/api/netbuf.c \
../Middlewares/Third_Party/lwIP/src/api/netdb.c \
../Middlewares/Third_Party/lwIP/src/api/netifapi.c \
../Middlewares/Third_Party/lwIP/src/api/sockets.c \
../Middlewares/Third_Party/lwIP/src/api/tcpip.c 

OBJS += \
./Middlewares/Third_Party/lwIP/src/api/api_lib.o \
./Middlewares/Third_Party/lwIP/src/api/api_msg.o \
./Middlewares/Third_Party/lwIP/src/api/err.o \
./Middlewares/Third_Party/lwIP/src/api/netbuf.o \
./Middlewares/Third_Party/lwIP/src/api/netdb.o \
./Middlewares/Third_Party/lwIP/src/api/netifapi.o \
./Middlewares/Third_Party/lwIP/src/api/sockets.o \
./Middlewares/Third_Party/lwIP/src/api/tcpip.o 

C_DEPS += \
./Middlewares/Third_Party/lwIP/src/api/api_lib.d \
./Middlewares/Third_Party/lwIP/src/api/api_msg.d \
./Middlewares/Third_Party/lwIP/src/api/err.d \
./Middlewares/Third_Party/lwIP/src/api/netbuf.d \
./Middlewares/Third_Party/lwIP/src/api/netdb.d \
./Middlewares/Third_Party/lwIP/src/api/netifapi.d \
./Middlewares/Third_Party/lwIP/src/api/sockets.d \
./Middlewares/Third_Party/lwIP/src/api/tcpip.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/lwIP/src/api/%.o: ../Middlewares/Third_Party/lwIP/src/api/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Inc" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/STM32F4xx_HAL_Driver/Inc" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Drivers/CMSIS/Include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/system" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/lwip" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/ipv4" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/netif" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/posix" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/src/include/posix/sys" -I"D:/Users/Hogan/Workspace_1/LWIP_TCP/Middlewares/Third_Party/lwIP/system/arch"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


