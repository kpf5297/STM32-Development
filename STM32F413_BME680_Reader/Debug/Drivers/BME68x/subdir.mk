################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BME68x/bme68x.c 

OBJS += \
./Drivers/BME68x/bme68x.o 

C_DEPS += \
./Drivers/BME68x/bme68x.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BME68x/%.o Drivers/BME68x/%.su Drivers/BME68x/%.cyclo: ../Drivers/BME68x/%.c Drivers/BME68x/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F413xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"/Users/kevinfox/Code/STM32F413_BME680_Reader/Drivers/BME68x" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BME68x

clean-Drivers-2f-BME68x:
	-$(RM) ./Drivers/BME68x/bme68x.cyclo ./Drivers/BME68x/bme68x.d ./Drivers/BME68x/bme68x.o ./Drivers/BME68x/bme68x.su

.PHONY: clean-Drivers-2f-BME68x

