################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include startup/subdir.mk
-include Src/subdir.mk
-include Middlewares/Third_Party/lwIP/src/netif/ppp/subdir.mk
-include Middlewares/Third_Party/lwIP/src/netif/subdir.mk
-include Middlewares/Third_Party/lwIP/src/core/ipv4/subdir.mk
-include Middlewares/Third_Party/lwIP/src/core/subdir.mk
-include Middlewares/Third_Party/lwIP/src/api/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: LWIP_TCP.elf

# Tool invocations
LWIP_TCP.elf: $(OBJS) $(USER_OBJS) ../STM32F407VETx_FLASH.ld
	@echo 'Building target: $@'
	@echo 'Invoking: MCU GCC Linker'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -specs=nosys.specs -specs=nano.specs -T"../STM32F407VETx_FLASH.ld" -Wl,-Map=output.map -Wl,--gc-sections -o "LWIP_TCP.elf" @"objects.list" $(USER_OBJS) $(LIBS) -lm
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

post-build:
	-@echo 'Generating binary and Printing size information:'
	arm-none-eabi-objcopy -O binary "LWIP_TCP.elf" "LWIP_TCP.bin"
	arm-none-eabi-size "LWIP_TCP.elf"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
