/*
 * XPT2046.h
 *
 *  Created on: May 26, 2025
 *      Author: kevinfox
 */

// xpt2046_lvgl.c
#include "XPT2046.h"

#define XPT2046_CMD_X 0xD0
#define XPT2046_CMD_Y 0x90

static SPI_HandleTypeDef *xpt_spi;

static void XPT2046_Select(void) {
    HAL_GPIO_WritePin(XPT2046_CS_PORT, XPT2046_CS_PIN, GPIO_PIN_RESET);
}

static void XPT2046_Unselect(void) {
    HAL_GPIO_WritePin(XPT2046_CS_PORT, XPT2046_CS_PIN, GPIO_PIN_SET);
}

static uint16_t XPT2046_Read(uint8_t cmd) {
    uint8_t buf[3] = { cmd, 0, 0 };
    uint8_t recv[3];

    XPT2046_Select();
    HAL_SPI_TransmitReceive(xpt_spi, buf, recv, 3, HAL_MAX_DELAY);
    XPT2046_Unselect();

    return ((recv[1] << 8) | recv[2]) >> 3; // 12-bit resolution
}

void XPT2046_Init(SPI_HandleTypeDef *hspi) {
    xpt_spi = hspi;
    XPT2046_Unselect();
}

bool XPT2046_TouchDetected(void) {
    // Optional: use IRQ or pressure threshold for better detection
    return true;
}

bool XPT2046_GetTouch(uint16_t *x, uint16_t *y) {
    if (!XPT2046_TouchDetected()) return false;

    *x = XPT2046_Read(XPT2046_CMD_X);
    *y = XPT2046_Read(XPT2046_CMD_Y);
    return true;
}

