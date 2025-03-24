/**
 * @file uart_commands.c
 * @author Kevin Fox
 * @brief  Implementation of UART command processing for STM32F446RE project
 * @version 0.1
 * @date 2025-03-23
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "uart_commands.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern TIM_HandleTypeDef htim2;

/**
 * @brief  Process the received command from UART.
 *
 * @param cmd
 */
void process_command(char *cmd)
{
    if (cmd == NULL || strlen(cmd) == 0)
    {
        printf("Empty command\r\n");
        return;
    }

    if (strncmp(cmd, CMD_SET_DUTY, strlen(CMD_SET_DUTY)) == 0)
    {

        const size_t prefixLen = strlen(CMD_SET_DUTY); // Usually = 1 for "D"
        if (strlen(cmd) > prefixLen)
        {
            // Parse the substring after 'D'
            float duty = atof(cmd + prefixLen);

            // Clip duty to 0..100 just in case
            if (duty < 0.0f)
                duty = 0.0f;
            if (duty > 100.0f)
                duty = 100.0f;

            uint32_t pwm_val = (uint32_t)((duty / 100.0f) * htim2.Init.Period);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_val);

            printf("Duty: %.1f%%\r\n", duty);
        }
        else
        {
            printf("No duty value provided\r\n");
        }
    }
    else if (strcmp(cmd, CMD_EMERGENCY_STOP) == 0)
    {
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
        printf("Emergency Stop!\r\n");
    }
    else
    {
        printf("Unknown command: %s\r\n", cmd);
    }
}
