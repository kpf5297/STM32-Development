/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
#define BUFFER_SIZE 64

uint8_t rxBuffer[BUFFER_SIZE]; // Buffer for receiving data
uint8_t TxData[BUFFER_SIZE]; // Buffer for transmitting data
uint8_t dataToSend[BUFFER_SIZE]; // Data to send
uint8_t dataReceived[BUFFER_SIZE]; // Data received from UART

uint8_t dataToSendLength = 0; // Length of the data to send
uint8_t dataReceivedIndex = 0; // Index for the received data
uint8_t isSent = 1; // Flag to check if data is sent
int currentSend = 0; // Current number of bytes sent
int maxSend = 1024 * 100; // Maximum number of bytes to send

uint16_t ADCValue = 0; // Variable to store ADC value
uint8_t buttonPushFlag = 0; // Flag to check if button is pushed

static SemaphoreHandle_t xPrintfSemaphore = NULL;
static SemaphoreHandle_t xSemaphore = NULL; // Semaphore for protecting shared resource

int BlueLED_TaskProfier = 0; // Task profiler variable
int Uart_TaskProfier = 0; // Task profiler variable
int YellowLED_TaskProfier = 0; // Task profiler variable
int ADCTaskProfier = 0; // Task profiler variable
int pushButtonTaskProfier = 0; // Task profiler variable
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
            HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY); // Transmit the character over UART
            xSemaphoreGive(xPrintfSemaphore);
        }
    }
    return ch;
}

uint8_t GetPushButtonFlag(void)
{
    uint8_t flag = 0;
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
    {
        flag = buttonPushFlag; // Get the button push flag
        xSemaphoreGive(xSemaphore);
    }
    return flag;
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    for (int i = 0; i < (int)sizeof(TxData) / 2; i++)
    {
      TxData[i] = i % 256;  
    }
    currentSend += sizeof(TxData) / 2; // Update the current send size
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  for (int i = (int)sizeof(TxData) / 2; i < (int)sizeof(TxData); i++)
  {
    TxData[i] = i % 256;
  }
  currentSend += sizeof(TxData) / 2; // Update the current send size
  isSent = 1; // Reset the flag to indicate that data can be sent again

  if (currentSend >= maxSend) // Check if the maximum send size has been reached
  {
    HAL_UART_DMAStop(&huart1); // Stop the DMA transfer
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (dataReceivedIndex < BUFFER_SIZE) // Check if the buffer is not full
  {
    dataReceived[dataReceivedIndex++] = rxBuffer[0]; // Store the received byte in the buffer
    HAL_UART_Receive_DMA(&huart1, &rxBuffer[0], 1); // Start receiving the next byte
  }
}
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  for (int i = 0; i < (int)sizeof(rxBuffer) / 2; i++)
  {
    dataReceived[dataReceivedIndex++] = rxBuffer[i]; // Store the received bytes in the buffer
  }
}

void SendData(uint8_t *data, uint16_t length) {
  if (length > 0 && data != NULL) // Check if the length is greater than 0 and data is not NULL
  {
    HAL_UART_Transmit_DMA(&huart1, data, length); // Transmit the data over UART using DMA
  }
}

// void HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin)
// {
//   if (GPIO_Pin == Push_Button_Pin) // Check if the interrupt is from the push button
//   {
//     // Semophore to protect the shared resource buttonPushFlag
//     if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
//     {
//         buttonPushFlag = 1; // Set the button push flag
//         xSemaphoreGive(xSemaphore); // Release the semaphore
//     }
//   }
// }

static void BlueLED_Task(void *argument) {
  for(;;) {
      // If ADC value is greater than 1000, torun on the blue LED, else turn it off
      if (ADCValue > 1000) {
          HAL_GPIO_WritePin(Blue_LED_GPIO_Port, Blue_LED_Pin, GPIO_PIN_SET); // Turn on blue LED
      } else {
          HAL_GPIO_WritePin(Blue_LED_GPIO_Port, Blue_LED_Pin, GPIO_PIN_RESET); // Turn off blue LED
      }

      BlueLED_TaskProfier++; // Increment task profiler variable
  }
}

static void YellowLED_Task(void *argument) {
  for(;;) {
      HAL_GPIO_TogglePin(Yellow_LED_GPIO_Port, Yellow_LED_Pin); // Toggle LED
      
      vTaskDelay(pdMS_TO_TICKS(5000)); // Delay for 500 milliseconds

      YellowLED_TaskProfier++; // Increment task profiler variable
  }
}

static void Uart_Task(void *argument) {
  for(;;) {
    
      if (isSent) // Check if data can be sent
      {
          HAL_UART_Transmit_DMA(&huart1, TxData, sizeof(TxData)); // Transmit data over UART using DMA
          isSent = 0; // Reset the flag to indicate that data is being sent
      }

      if (dataReceivedIndex > 0) // Check if there is data in the buffer
      {
          for (int i = 0; i < dataReceivedIndex; i++)
          {
              printf("Received: %c\n", dataReceived[i]); // Print the received data
          }
          dataReceivedIndex = 0; // Reset the index after processing the received data
      }

      Uart_TaskProfier++; // Increment task profiler variable
      vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 milliseconds
  }
}

static void ADCTask(void *argument) {
  for(;;) {
      HAL_ADC_Start(&hadc1); // Start ADC conversion
      HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY); // Poll for conversion completion
      ADCValue = HAL_ADC_GetValue(&hadc1); // Get the ADC value
      HAL_ADC_Stop(&hadc1); // Stop ADC conversion
      printf("ADC Value: %d\n", ADCValue); // Print the ADC value
      ADCTaskProfier++; // Increment task profiler variable
  }
}

static void pushButtonTask(void *argument) {
  for(;;) {
      if (GetPushButtonFlag()) // Check if the button is pushed
      {
          HAL_GPIO_TogglePin(Yellow_LED_GPIO_Port, Yellow_LED_Pin); // Toggle LED
          buttonPushFlag = 0; // Reset the button push flag
      }
      pushButtonTaskProfier++; // Increment task profiler variable
  }
}

void tasks_init(void) {
  
  
if (xTaskCreate(BlueLED_Task, "BlueLED_Task", 128, NULL, 1, NULL) != pdPASS) {
  // Handle error: BlueLED_Task creation failed
  printf("Error: BlueLED_Task creation failed\n");
}

if (xTaskCreate(YellowLED_Task, "YellowLED_Task", 128, NULL, 1, NULL) != pdPASS) {
  // Handle error: YellowLED_Task creation failed
  printf("Error: YellowLED_Task creation failed\n");
}

if (xTaskCreate(Uart_Task, "Uart_Task", 128, NULL, 1, NULL) != pdPASS) {
  // Handle error: Uart_Task creation failed
  printf("Error: Uart_Task creation failed\n");
}

if (xTaskCreate(pushButtonTask, "pushButtonTask", 128, NULL, 1, NULL) != pdPASS) {
  // Handle error: pushButtonTask creation failed
  printf("Error: pushButtonTask creation failed\n");
}
if (xTaskCreate(ADCTask, "ADCTask", 256, NULL, 1, NULL) != pdPASS) {
  // Handle error: ADCTask creation failed
  printf("Error: ADCTask creation failed\n");
}

  xPrintfSemaphore = xSemaphoreCreateMutex(); // Create a mutex for printf
  if (xPrintfSemaphore == NULL) {
      // Handle error
  }
  xSemaphore = xSemaphoreCreateMutex(); // Create a mutex for protecting shared resource
  if (xSemaphore == NULL) {
      // Handle error
  }

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  // Start receiving data in DMA mode
  HAL_UART_Receive_DMA(&huart1, rxBuffer, 1);
  // Initialize data to send
  dataToSend[0] = 'H';
  dataToSend[1] = 'e';
  dataToSend[2] = 'l';  
  dataToSend[3] = 'l';
  dataToSend[4] = 'o';
  dataToSend[5] = '\0'; // Null-terminate the string
  dataToSendLength = 5; // Length of the data to send
  // Start transmitting data in DMA mode
  // HAL_UART_Transmit_DMA(&huart1, dataToSend, dataToSendLength);

  SendData(dataToSend, dataToSendLength); // Send data over UART using DMA

  // Ensure clock for push button is enabled

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */

  /* USER CODE BEGIN RTOS_THREADS */
  tasks_init(); // Initialize tasks here if needed
  vTaskStartScheduler(); // Start the FreeRTOS scheduler
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Blue_LED_Pin|Yellow_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Blue_LED_Pin Yellow_LED_Pin */
  GPIO_InitStruct.Pin = Blue_LED_Pin|Yellow_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : Push_Button_Pin */
  GPIO_InitStruct.Pin = Push_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Push_Button_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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

#ifdef  USE_FULL_ASSERT
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
