#include "tasks.h"

// Standard library includes
#include <stdio.h>   // Needed for printf()
#include <stdlib.h>  // Needed for atof()

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// HAL includes
#include "stm32f4xx_hal.h"
#include "config.h"
#include "main.h"   // So we can get LED_Pin, etc.

// -------------------------------------------------------------------
// 1) EXTERN declarations so we can use them here:
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart2;
extern QueueHandle_t uartRxQueue;
// -------------------------------------------------------------------

// Local (static) handles, semaphores, etc.
static SemaphoreHandle_t xPrintfSemaphore = NULL;

// Task handles
static TaskHandle_t task1_handle       = NULL;
static TaskHandle_t task2_handle       = NULL;
static TaskHandle_t xCommandTaskHandle = NULL;

// Example “profiling” counters
static uint32_t task1Profiler = 0;
static uint32_t task2Profiler = 0;
static uint32_t uartProfiler  = 0;

// Forward declarations
static void Task1(void *pvParameter);
static void Task2(void *pvParameter);
static void uartProcessingTask(void *pvParameter);

// If you want to redirect printf -> UART
int __io_putchar(int ch)
{
    if (xPrintfSemaphore != NULL)
    {
        // Wait for the semaphore
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
    // Create the mutex for printf
    xPrintfSemaphore = xSemaphoreCreateMutex();
    configASSERT(xPrintfSemaphore);

    // Create the queue for UART
    uartRxQueue = xQueueCreate(UART_RX_QUEUE_LENGTH, sizeof(uint8_t));
    configASSERT(uartRxQueue);

    // Start receiving interrupts
    uint8_t receiveByte;
    HAL_UART_Receive_IT(&huart2, &receiveByte, 1);

    // Create Task1
    xTaskCreate(Task1,
                "Task1",
                TASK_STACK_SIZE_SMALL,
                NULL,
                TASK_PRIORITY_LOW,
                &task1_handle);

    // Create Task2
    xTaskCreate(Task2,
                "Task2",
                TASK_STACK_SIZE_SMALL,
                NULL,
                TASK_PRIORITY_LOW,
                &task2_handle);

    // Create UART Processing Task
    xTaskCreate(uartProcessingTask,
                "UARTTask",
                TASK_STACK_SIZE_SMALL,
                NULL,
                TASK_PRIORITY_MEDIUM,
                &xCommandTaskHandle);
}

/**
  * @brief  Example Task1: toggles LED, prints
  */
static void Task1(void *pvParameter)
{
    for (;;)
    {
        printf("Task1 Running\r\n");
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);  // LED blink
        task1Profiler++;
        vTaskDelay(WAIT_TIME_MEDIUM); // from config.h
    }
}

/**
  * @brief  Example Task2: prints, delays
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
  * @brief  UART Processing Task
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
                    uint32_t compareVal = (uint32_t)((dutyCycle / 100.0f) * (htim2.Init.Period));
                    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, compareVal);

                    printf("Duty: %.1f%%\r\n", dutyCycle);
                }
            }
            else
            {
                // Possibly check for 'E', 'ESTOP', etc.
                printf("Invalid Command\r\n");
            }
        }
        uartProfiler++;
    }
}
