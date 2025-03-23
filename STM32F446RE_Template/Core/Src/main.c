/*
  Author: Kevin Fox
  Date: 23 Mar

  Goal: [App] ↔ [Bluetooth/USB/UART Bridge] ↔ [STM32F446RE] → [Koford S24V10A-4A] → [Motor]

  Frequency: 37 kHz, as recommended by Koford datasheet

  Set Duty Cycle:
  Command: "DXX.X"
  Example: "D50.5" sets the duty cycle to 50.5%
  The command starts with 'D' followed by a floating-point number (0.0 to 100.0).
  This adjusts the PWM output to control motor speed.

  Emergency Stop:
  Command: "ESTOP"
  This immediately stops the PWM output to the motor.

  Protocol:
  - ASCII commands terminated by a newline ('\r' or '\n').
  - Example: To set motor speed to 75%, send: "D75.0\r\n".
             To perform an emergency stop, send: "ESTOP\r\n".

  Responses:
  - "Duty: XX.X%" after setting a new duty cycle.
  - "Emergency Stop!" after executing an emergency stop.
*/

#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "uart_commands.h"
#include "config.h"

// Handles
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart2;

// Initialization functions
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void const *argument);

// UART printf redirection
SemaphoreHandle_t xPrintfSemaphore;
int __io_putchar(int ch);                                // Redirect printf to UART

// Task handles for future use/profiling
TaskHandle_t task1_handle, task2_handle, xCommandTaskHandle;

 typedef uint32_t TaskProfiler;
 TaskProfiler task1Profiler, task2Profiler, uartProfiler;

void Task1(void *pvParameter);
void Task2(void *pvParameter);

QueueHandle_t uartRxQueue;
void uartProcessingTask(void *pvParameters);             // UART processing task
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart); // UART RX complete callback

int main(void)
{

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  xPrintfSemaphore = xSemaphoreCreateMutex();
  configASSERT(xPrintfSemaphore);

  xTaskCreate(Task1, "Task1", TASK_STACK_SIZE_SMALL, NULL, TASK_PRIORITY_LOW, &task1_handle);
  xTaskCreate(Task2, "Task2", TASK_STACK_SIZE_SMALL, NULL, TASK_PRIORITY_LOW, &task2_handle);

  uartRxQueue = xQueueCreate(UART_RX_QUEUE_LENGTH, sizeof(uint8_t));

  xTaskCreate(uartProcessingTask, "UARTTask", TASK_STACK_SIZE_SMALL, NULL, TASK_PRIORITY_MEDIUM, NULL);

  uint8_t receiveByte;
  HAL_UART_Receive_IT(&huart2, &receiveByte, 1);

  vTaskStartScheduler();

  while (1)
  {
  }
}

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

void Task1(void *pvParameter)
{
  while (1)
  {
    printf("Task1 Running\n\r");
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    task1Profiler++;
    vTaskDelay(WAIT_TIME_MEDIUM); // Simulate work
  }
}

void Task2(void *pvParameter)
{
  while (1)
  {
    printf("Task2 Running\n\r");
    HAL_Delay(200); // Simulate work
    task2Profiler++;
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  uint8_t receivedByte;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (huart->Instance == USART2)
  {
    receivedByte = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
    xQueueSendFromISR(uartRxQueue, &receivedByte, &xHigherPriorityTaskWoken);
    HAL_UART_Receive_IT(huart, &receivedByte, 1);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void uartProcessingTask(void *pvParameters)
{
  uint8_t receivedByte;

  while (1)
  {
    if (xQueueReceive(uartRxQueue, &receivedByte, portMAX_DELAY) == pdPASS)
    {
      // Process received byte
      if (receivedByte == 'D') // Start of duty cycle command
      {
        char command[10];
        int index = 0;

        // Read until newline or buffer full
        while (index < sizeof(command) - 1)
        {
          if (xQueueReceive(uartRxQueue, &receivedByte, portMAX_DELAY) == pdPASS)
          {
            if (receivedByte == '\r' || receivedByte == '\n')
            {
              break; // End of command
            }
            command[index++] = receivedByte;
          }
        }
        command[index] = '\0'; // Null-terminate the string

        // Set duty cycle
        float dutyCycle = atof(command);
        if (dutyCycle >= 0.0f && dutyCycle <= 100.0f)
        {
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (uint32_t)((dutyCycle / 100.0f) * (htim2.Init.Period)));
          printf("Duty: %.1f%%\n\r", dutyCycle);
        }
      }
      else if (strncmp((char *)&receivedByte, "ESTOP", 5) == 0)
      {
        // Emergency stop
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0); // Stop PWM
        printf("Emergency Stop!\n\r");
      }
      else
      {
        // Handle other commands or invalid input
        printf("Invalid Command\n\r");
      }
    }
    uartProfiler++;
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  // Calc timer period for PWM frequency
  uint32_t timerClock = HAL_RCC_GetPCLK1Freq() * 2;   // APB1
  uint32_t period = (timerClock / PWM_FREQUENCY) - 1; // counts based on timer clock

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = period;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;

  uint32_t pulse = (PWM_DUTY_CYCLE_INITIAL / 100.0f) * (period);

  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim2);
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for (;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}
/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
