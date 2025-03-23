/*
 * config.h
 *
 *  Created on: Mar 23, 2025
 *      Author: kevinfox
 */

#ifndef CONFIG_H
#define CONFIG_H

// Task priorities
#define TASK_PRIORITY_LOW 1
#define TASK_PRIORITY_MEDIUM 2
#define TASK_PRIORITY_HIGH 3

// Task stack sizes
#define TASK_STACK_SIZE_SMALL 128
#define TASK_STACK_SIZE_MEDIUM 256
#define TASK_STACK_SIZE_LARGE 512

// Wait times
#define WAIT_TIME_SHORT pdMS_TO_TICKS(100)
#define WAIT_TIME_MEDIUM pdMS_TO_TICKS(500)
#define WAIT_TIME_LONG pdMS_TO_TICKS(1000)

// UART configuration
#define UART_BAUD_RATE 115200
#define UART_RX_QUEUE_LENGTH 128
#define UART_RX_BUFFER_SIZE 64

// PWM configuration
#define PWM_FREQUENCY 1000 // Hz
#define PWM_RESOLUTION 255 // 8-bit resolution

#endif // CONFIG_H

