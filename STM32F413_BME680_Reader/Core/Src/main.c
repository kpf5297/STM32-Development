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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "bme68x.h"

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
I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
I2C_HandleTypeDef hi2c2;   // I2C2 handler
UART_HandleTypeDef huart6; // UART6 handler

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */
void I2C_Scan(I2C_HandleTypeDef *hi2c);
int8_t bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
void bme68x_delay_us(uint32_t period_us, void *intf_ptr);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_I2C2_Init();
  MX_USART6_UART_Init();

  /* USER CODE BEGIN 2 */
  uint8_t chip_id = 0;
  HAL_StatusTypeDef status;

  status = HAL_I2C_Mem_Read(&hi2c2, (BME68X_I2C_ADDR_HIGH << 1), 0xD0, I2C_MEMADD_SIZE_8BIT, &chip_id, 1, HAL_MAX_DELAY);

  if (status == HAL_OK)
  {
    char msg[50];
    sprintf(msg, "BME680 Chip ID: 0x%02X\r\n", chip_id);
    HAL_UART_Transmit(&huart6, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
  }
  else
  {
    char error_msg[] = "Failed to read Chip ID\r\n";
    HAL_UART_Transmit(&huart6, (uint8_t *)error_msg, strlen(error_msg), HAL_MAX_DELAY);
  }

  I2C_Scan(&hi2c2);

  struct bme68x_dev bme680;
  int8_t result;
  struct bme68x_conf conf;
  struct bme68x_heatr_conf heatr_conf;

  uint8_t dev_addr = BME68X_I2C_ADDR_HIGH; // Use 0x77
  bme680.chip_id = BME68X_CHIP_ID;         // Set to 0x61
  bme680.intf_ptr = &dev_addr;             // Pass I2C address via intf_ptr
  bme680.intf = BME68X_I2C_INTF;
  bme680.read = bme68x_i2c_read;
  bme680.write = bme68x_i2c_write;
  bme680.delay_us = bme68x_delay_us;

  result = bme68x_init(&bme680);

  HAL_Delay(1000);  // remove ?? 
  // Initialize the sensor
  if (result != BME68X_OK)
  {
    char error_msg[] = "BME680 Initialization Failed\r\n";
    HAL_UART_Transmit(&huart6, (uint8_t *)error_msg, strlen(error_msg), HAL_MAX_DELAY);
    while (1)
      ; // Halt on failure
  }

  conf.filter = BME68X_FILTER_OFF;
  // conf.odr = BME68X_ODR_NONE;  // verify data sheet
  conf.os_hum = BME68X_OS_16X;
  conf.os_temp = BME68X_OS_2X;
  conf.os_pres = BME68X_OS_1X;

  result = bme68x_set_conf(&conf, &bme680);
  if (result != BME68X_OK)
  {
    char error_msg[] = "BME680 Config Failed\r\n";
    HAL_UART_Transmit(&huart6, (uint8_t *)error_msg, strlen(error_msg), HAL_MAX_DELAY);
    while (1)
      ; // Halt on failure
  }

  heatr_conf.enable = BME68X_ENABLE;
  heatr_conf.heatr_temp = 300; // 300C !!!!! (for VOC need to read)
  heatr_conf.heatr_dur = 100;  // 100ms

  //  Keeing heater off since not using VOC
  //  if (bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme680) != BME68X_OK) {
  //      char error_msg[] = "BME680 HEater Config Failed\r\n";
  //      HAL_UART_Transmit(&huart6, (uint8_t *)error_msg, strlen(error_msg), HAL_MAX_DELAY);
  //      while (1); // Halt on failure
  //  }

  float temp_offset = 0.0;
  float hum_offset = 0.0;
  struct bme68x_data data;
  uint32_t del_period;
  uint8_t n_fields;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // Trigger measurement in forced mode
    if (bme68x_set_op_mode(BME68X_FORCED_MODE, &bme680) != BME68X_OK) {
        HAL_UART_Transmit(&huart6, (uint8_t *)"Set op mode failed\r\n", 20, HAL_MAX_DELAY);
        continue;
    }

    // Calculate delay period if heater is enabled.  Library will be added to process data later.
    del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme680) + (heatr_conf.heatr_dur * 1000);
    HAL_Delay(del_period / 1000); // Wait for measurement

    // Read data
    if (bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme680) == BME68X_OK) {
        float temperature = (data.temperature) + temp_offset;
        float temp_f = (temperature * 9.0f / 5.0f) + 32.0f;
        float humidity = (data.humidity) + hum_offset;
        float gas_resistance = data.gas_resistance / 1000.0f; // Convert to kOhm

        char uart_buffer[150];
        sprintf(uart_buffer, "Temp: %.2f C (%.2f F) | Hum: %.2f %% | Press: %.2f hPa | Gas: %.2f kOhm\r\n",
                        temperature, temp_f, humidity, data.pressure / 100.0f, gas_resistance);
        HAL_UART_Transmit(&huart6, (uint8_t *)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);
    }

    HAL_Delay(2000);

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

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief I2C2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */
}

/**
 * @brief USART6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
int8_t bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
	uint8_t dev_addr = *(uint8_t *)intf_ptr;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c2, (uint16_t)(dev_addr << 1), reg_addr,
                                                I2C_MEMADD_SIZE_8BIT, reg_data, len, HAL_MAX_DELAY);
    return (status == HAL_OK) ? 0 : -1;
}

int8_t bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
	uint8_t dev_addr = *(uint8_t *)intf_ptr;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c2, (uint16_t)(dev_addr << 1), reg_addr,
                                                 I2C_MEMADD_SIZE_8BIT, (uint8_t *)reg_data, len,
                                                 HAL_MAX_DELAY);
    return (status == HAL_OK) ? 0 : -1;
}

void bme68x_delay_us(uint32_t period_us, void *intf_ptr) {
    HAL_Delay(period_us / 1000); // Convert microseconds to milliseconds
}

void I2C_Scan(I2C_HandleTypeDef *hi2c) {
  char uart_buffer[50];
  HAL_UART_Transmit(&huart6, (uint8_t *)"Scanning I2C bus...\r\n", strlen("Scanning I2C bus...\r\n"), HAL_MAX_DELAY);

  for (uint16_t i = 0; i < 128; i++) {
      if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(i << 1), 1, 10) == HAL_OK) {
          sprintf(uart_buffer, "Device found at 0x%02X\r\n", i);
          HAL_UART_Transmit(&huart6, (uint8_t *)uart_buffer, strlen(uart_buffer), HAL_MAX_DELAY);
      }
  }

  HAL_UART_Transmit(&huart6, (uint8_t *)"I2C scan complete.\r\n", strlen("I2C scan complete.\r\n"), HAL_MAX_DELAY);
}

/* USER CODE END 4 */

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
