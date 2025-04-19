#include "systasks.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart2;
extern uint8_t button_pressed; // Flag to indicate button press

void task1(void *pvParameters) {
    // Task 1 implementation
    while (1) {
        // Increment the counter
        task1_counter++;
        // Delay for 2000 ms
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void pwm_task(void *pvParameters) {
    uint8_t lightOn = 0;
    // PWM task implementation
    while (1) {
        // Check if the button is pressed
        if (button_pressed) {
            lightOn = !lightOn; // Toggle the light state
            if (lightOn) {
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 1000); // Set duty cycle to 50%
            } else {
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); // Set duty cycle to 0%
            }
            button_pressed = 0; // Reset the flag
        }

        // Increment the counter
        pwm_task_counter++;
        // Delay for 1000 ms
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void blink_task(void *pvParameters) {
    // Blink task implementation
    while (1) {
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); // Toggle the LE
        // Increment the counter
        blink_task_counter++;
        // Delay for 500 ms
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
void tasks_init(void) {
    // Create the tasks
    xTaskCreate(task1, "Task1", 128, NULL, 1, NULL);
    xTaskCreate(pwm_task, "PWM Task", 128, NULL, 2, NULL);
    xTaskCreate(blink_task, "Blink Task", 128, NULL, 3, NULL);
}