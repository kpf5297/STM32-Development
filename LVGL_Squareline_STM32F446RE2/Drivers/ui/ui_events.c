// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.2
// LVGL version: 9.2.2
// Project name: project_02

#include "ui.h"

static const uint32_t palette[] = {
    0x003a57, 0x00FF00, 0xFF0000,
    0x0000FF, 0x123456, 0x0000ff
};

void background_color_clicked(lv_event_t * e)
{
    printf("button1_clicked()\r\n");        /* debug proof */

    static uint8_t idx = 0;                 /* cycles 0‥5 */

    lv_obj_set_style_bg_color(ui_MainScreen,
                              lv_color_hex(palette[idx]),
                              LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_opa  (ui_MainScreen,
                              LV_OPA_COVER,
                              LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_invalidate(ui_MainScreen);          /* <— key line */

    idx = (idx + 1) % (sizeof(palette)/sizeof(palette[0]));
}
