
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"



void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void Error_Handler(void);

#define LED_Pin GPIO_PIN_5
#define LED_GPIO_Port GPIOA

#define UART_RX_QUEUE_LENGTH 128
#define UART_RX_BUFFER_SIZE 64

#define configUSE_PREEMPTION 1


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
