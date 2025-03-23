#include "uart_commands.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern TIM_HandleTypeDef htim2;

void process_command(char *cmd)
{
    // Sanity-check the incoming string
    if (cmd == NULL || strlen(cmd) == 0)
    {
        printf("Empty command\r\n");
        return;
    }

    // Check if command starts with D (Set duty)
    if (strncmp(cmd, CMD_SET_DUTY, strlen(CMD_SET_DUTY)) == 0)
    {
        // e.g. cmd = "D50.5" -> skip the "D" to parse "50.5"
        const size_t prefixLen = strlen(CMD_SET_DUTY);  // Usually = 1 for "D"
        if (strlen(cmd) > prefixLen)
        {
            // Parse the substring after 'D'
            float duty = atof(cmd + prefixLen);

            // Clip duty to 0..100 just in case
            if (duty < 0.0f)   duty = 0.0f;
            if (duty > 100.0f) duty = 100.0f;

            // Convert %duty into a timer compare value
            uint32_t pwm_val = (uint32_t)((duty / 100.0f) * htim2.Init.Period);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_val);

            printf("Duty: %.1f%%\r\n", duty);
        }
        else
        {
            // The user typed just "D" with nothing after it
            printf("No duty value provided\r\n");
        }
    }
    else if (strcmp(cmd, CMD_EMERGENCY_STOP) == 0)
    {
        // e.g. cmd = "ESTOP"
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
        printf("Emergency Stop!\r\n");
    }
    else
    {
        // Unknown
        printf("Unknown command: %s\r\n", cmd);
    }
}
