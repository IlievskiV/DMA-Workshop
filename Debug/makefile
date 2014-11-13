################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: DMA_Workshop.axf

# Tool invocations
DMA_Workshop.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\Users\Vladimir\Documents\LPCXpresso_7.1.1_125\workspace\CMSIS_CORE_LPC17xx\Debug" -Xlinker -Map="DMA_Workshop.map" -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T "DMA_Workshop_Debug.ld" -o "DMA_Workshop.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) DMA_Workshop.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "DMA_Workshop.axf"; # arm-none-eabi-objcopy -O binary "DMA_Workshop.axf" "DMA_Workshop.bin" ; checksum -p LPC1769 -d "DMA_Workshop.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
