/*
 * XPT2046.h
 *
 *  Created on: May 26, 2025
 *      Author: kevinfox
 */
// xpt2046.h
#ifndef XPT2046_H
#define XPT2046_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

#define XPT2046_CS_PORT GPIOA
#define XPT2046_CS_PIN  GPIO_PIN_8

void XPT2046_Init(SPI_HandleTypeDef *hspi);
bool XPT2046_TouchDetected(void);
bool XPT2046_GetTouch(uint16_t *x, uint16_t *y);

#endif // XPT2046_H



