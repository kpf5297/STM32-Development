/**
 * @file tasks.c
 * @author Kevin Fox
 * @brief FreeRTOS tasks implementation for STM32F446RE project
 * @version 0.1
 * @date 2025-03-23
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "tasks.h"
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "stm32f4xx_hal.h"
#include "config.h"
#include "main.h"

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart2;
extern QueueHandle_t uartRxQueue;

static SemaphoreHandle_t xPrintfSemaphore = NULL;

static TaskHandle_t task1_handle = NULL;
static TaskHandle_t task2_handle = NULL;
static TaskHandle_t xCommandTaskHandle = NULL;

static uint32_t task1Profiler = 0;
static uint32_t task2Profiler = 0;
static uint32_t uartProfiler = 0;

static void Task1(void *pvParameter);
static void Task2(void *pvParameter);
static void uartProcessingTask(void *pvParameter);

/**
 * @brief  UART receive complete callback
 *         This function is called when a byte is received over UART.
 *         It sends the received byte to the FreeRTOS queue for processing.
 * @param  huart: pointer to the UART handle
 */
int __io_putchar(int ch)
{
    if (xPrintfSemaphore != NULL)
    {
        if (xSemaphoreTake(xPrintfSemaphore, portMAX_DELAY) == pdTRUE)
        {
            HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
            xSemaphoreGive(xPrintfSemaphore);
        }
    }
    return ch;
}

/**
 * @brief  Create and start all tasks, queues, semaphores, etc.
 */
void tasks_init(void)
{
    xPrintfSemaphore = xSemaphoreCreateMutex();
    configASSERT(xPrintfSemaphore);

    uartRxQueue = xQueueCreate(UART_RX_QUEUE_LENGTH, sizeof(uint8_t));
    configASSERT(uartRxQueue);

    uint8_t receiveByte;
    HAL_UART_Receive_IT(&huart2, &receiveByte, 1);

    xTaskCreate(Task1,
                "Task1",
                TASK_STACK_SIZE_SMALL,
                NULL,
                TASK_PRIORITY_LOW,
                &task1_handle);

    xTaskCreate(Task2,
                "Task2",
                TASK_STACK_SIZE_SMALL,
                NULL,
                TASK_PRIORITY_LOW,
                &task2_handle);

    xTaskCreate(uartProcessingTask,
                "UARTTask",
                TASK_STACK_SIZE_SMALL,
                NULL,
                TASK_PRIORITY_MEDIUM,
                &xCommandTaskHandle);
}

/**
 * @brief  Example Task1: prints every WAIT_TIME_MEDIUM ms
 *         and toggles an LED.
 */
static void Task1(void *pvParameter)
{
    for (;;)
    {
        printf("Task1 Running\r\n");
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); // LED blink
        task1Profiler++;
        vTaskDelay(WAIT_TIME_MEDIUM); // from config.h
    }
}

/**
 * @brief  Example Task2: prints every 200ms
 */
static void Task2(void *pvParameter)
{
    for (;;)
    {
        printf("Task2 Running\r\n");

        HAL_Delay(200);
        task2Profiler++;
    }
}

/**
 * @brief  UART processing task: processes received commands from the queue
 *         and sets PWM duty cycle based on the command.
 */
static void uartProcessingTask(void *pvParameter)
{
    uint8_t receivedByte;
    for (;;)
    {
        if (xQueueReceive(uartRxQueue, &receivedByte, portMAX_DELAY) == pdPASS)
        {
            if (receivedByte == 'D')
            {
                // Collect rest of command
                char command[10];
                int index = 0;

                while (index < sizeof(command) - 1)
                {
                    if (xQueueReceive(uartRxQueue, &receivedByte, portMAX_DELAY) == pdPASS)
                    {
                        if (receivedByte == '\r' || receivedByte == '\n')
                            break;
                        command[index++] = receivedByte;
                    }
                }
                command[index] = '\0';

                // Convert string to float for duty cycle
                float dutyCycle = atof(command);
                if (dutyCycle >= 0.0f && dutyCycle <= 100.0f)
                {

                    uint32_t compareVal = (uint32_t)((dutyCycle / 100.0f) * (htim2.Init.Period + 1));
                    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, compareVal);

                    printf("Duty Cycle Set: %.2f%%\r\n", dutyCycle);
                }
            }
            else
            {
                printf("Invalid Command\r\n");
            }
        }
        uartProfiler++;
    }
}
