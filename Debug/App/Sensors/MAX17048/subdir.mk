################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../App/Sensors/MAX17048/MAX17048.cpp 

OBJS += \
./App/Sensors/MAX17048/MAX17048.o 

CPP_DEPS += \
./App/Sensors/MAX17048/MAX17048.d 


# Each subdirectory must supply rules for building sources it contributes
App/Sensors/MAX17048/%.o App/Sensors/MAX17048/%.su App/Sensors/MAX17048/%.cyclo: ../App/Sensors/MAX17048/%.cpp App/Sensors/MAX17048/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32WB55xx -c -I../Core/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../Drivers/CMSIS/Include -I../USB_Device/App -I../USB_Device/Target -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/RTOS2/Include -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../App -I../App/Tasks -I../App/Interfaces -I../App/Sensors -I../App/Sensors/MAX30102 -I../App/Sensors/KX126 -I../App/Sensors/MAX17048 -I../STM32_WPAN/App -I../Utilities/lpm/tiny_lpm -I../Utilities/sequencer -I../Middlewares/ST/STM32_WPAN -I../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../Middlewares/ST/STM32_WPAN/utilities -I../Middlewares/ST/STM32_WPAN/ble/core -I../Middlewares/ST/STM32_WPAN/ble/core/auto -I../Middlewares/ST/STM32_WPAN/ble/core/template -I../Middlewares/ST/STM32_WPAN/ble/svc/Inc -I../Middlewares/ST/STM32_WPAN/ble/svc/Src -I../Middlewares/ST/STM32_WPAN/ble -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-Sensors-2f-MAX17048

clean-App-2f-Sensors-2f-MAX17048:
	-$(RM) ./App/Sensors/MAX17048/MAX17048.cyclo ./App/Sensors/MAX17048/MAX17048.d ./App/Sensors/MAX17048/MAX17048.o ./App/Sensors/MAX17048/MAX17048.su

.PHONY: clean-App-2f-Sensors-2f-MAX17048

