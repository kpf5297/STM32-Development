/*
 * uart_commands.c
 *
 *  Created on: Mar 23, 2025
 *      Author: kevinfox
 */

#include "uart_commands.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern TIM_HandleTypeDef htim2;

void process_command(char *cmd) {
    if (strncmp(cmd, CMD_SET_DUTY, strlen(CMD_SET_DUTY)) == 0) {
        float duty = atof(cmd + strlen(CMD_SET_DUTY));
        uint16_t pwm_val = (duty / 100.0f) * htim2.Init.Period;
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_val);
        printf("Duty: %.1f%%\r\n", duty);
    }
    else if (strcmp(cmd, CMD_EMERGENCY_STOP) == 0) {
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
        printf("Emergency Stop!\r\n");
    }
    else {
        printf("Unknown command: %s\r\n", cmd);
    }
}

