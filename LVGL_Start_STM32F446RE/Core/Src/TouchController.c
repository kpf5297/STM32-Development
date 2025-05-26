/*
 * TouchController.c
 *
 *  Created on: May 26, 2025
 *      Author: kevinfox
 */
#include "TouchController.h"
#include "XPT2046.h"
#include "main.h"

extern SPI_HandleTypeDef hspi2;

static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data);

void TouchController_Init(void) {
    XPT2046_Init(&hspi2);

    lv_indev_t *touch_indev = lv_indev_create();
    lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch_indev, touchpad_read);
}

static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    static int16_t last_x = 0;
    static int16_t last_y = 0;

    uint16_t x, y;
    if (XPT2046_GetTouch(&x, &y)) {
        last_x = x;
        last_y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    // Optional: map raw ADC range to screen resolution
    data->point.x = 240 - (last_y * 240 / 4095); // Calibrate as needed
    data->point.y = (last_x * 320 / 4095);
}
