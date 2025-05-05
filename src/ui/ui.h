#pragma once

#include "sys_init.h"

void init_ui(void);
void handle_ui(void);
void dsiplay_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map);
void ctp_read_cb(lv_indev_t * drv, lv_indev_data_t * data);
static uint32_t lv_tick_cb(void);