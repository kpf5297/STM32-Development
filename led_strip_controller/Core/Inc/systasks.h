// Tasks to be used in the FreeRTOS scheduler
#ifndef __SYSTASKS_H
#define __SYSTASKS_H

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

#ifdef __cplusplus
extern "C"
{
#endif

void task1(void *argument);
void pwm_task(void *argument);
void blink_task(void *argument);
void tasks_init(void);

static int task1_counter = 0;
static int pwm_task_counter = 0;
static int blink_task_counter = 0;

#ifdef __cplusplus
}
#endif

#endif // __SYSTASKS_H