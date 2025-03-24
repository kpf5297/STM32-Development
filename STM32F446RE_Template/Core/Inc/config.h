/**
 * @file config.h
 * @author Kevin Fox
 * @brief  Configuration file for STM32F446RE project
 * @version 0.1
 * @date 2025-03-23
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

#define TASK_PRIORITY_LOW 1
#define TASK_PRIORITY_MEDIUM 2
#define TASK_PRIORITY_HIGH 3

#define TASK_STACK_SIZE_SMALL 512
#define TASK_STACK_SIZE_MEDIUM 1024
#define TASK_STACK_SIZE_LARGE 2048

#define WAIT_TIME_SHORT pdMS_TO_TICKS(100)
#define WAIT_TIME_MEDIUM pdMS_TO_TICKS(500)
#define WAIT_TIME_LONG pdMS_TO_TICKS(1000)

#define UART_BAUD_RATE 115200
#define UART_RX_QUEUE_LENGTH 128
#define UART_RX_BUFFER_SIZE 64

#define PWM_FREQUENCY 37000 // 37 kHz per Koford datasheet
// #define PWM_RESOLUTION 255 // 8-bit resolution or 256 counts
#define PWM_DUTY_CYCLE_INITIAL 0

#endif // CONFIG_H
