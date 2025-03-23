/*
 * uart_commands.h
 *
 *  Created on: Mar 23, 2025
 *      Author: kevinfox
 */

#ifndef UART_COMMANDS_H
#define UART_COMMANDS_H

#include <stdint.h>

// Command definitions
#define CMD_SET_DUTY "D"
#define CMD_EMERGENCY_STOP "ESTOP"

// Function prototypes
void process_command(char *cmd);

#endif // UART_COMMANDS_H

