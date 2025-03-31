/**
 * @file uart_commands.h
 * @author Kevin Fox
 * @brief Header file for UART command processing in STM32F446RE project
 * @version 0.1
 * @date 2025-03-23
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef UART_COMMANDS_H
#define UART_COMMANDS_H

#include <stdint.h>

#define CMD_SET_DUTY "D"
#define CMD_EMERGENCY_STOP "ESTOP"

void process_command(char *cmd);

#endif // UART_COMMANDS_H
