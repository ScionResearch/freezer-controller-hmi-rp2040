#include "ui.h"

static uint16_t display_buf[(320 * 480)/10];

void init_ui(void) {
    // Init LVGL
    lv_init();
    //lv_log_register_print_cb(log_print_cb);
    lv_display_t * display = lv_display_create(320, 480);
    //if (debug) Serial.println("Display created and LVGL initialised");

    displayInit();

    // Register display and input device drivers
    lv_display_set_flush_cb(display, dsiplay_flush_cb);
    //if (debug) Serial.println("Flush callback registered");

    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, ctp_read_cb);
    lv_indev_set_long_press_time(indev, 2000); 
    //if (debug) Serial.println("CTP read callback registered");

    // Set screen draw buffers
    lv_display_set_buffers(display, display_buf, NULL, sizeof(display_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Register tick timer callback
    lv_tick_set_cb(lv_tick_cb);
    //if (debug) Serial.println("Tick callback registered");

    // Setup screen
    screenMainInit();
}

void handle_ui(void) {
    if (newSetpointReceived && !controlConfigLocked) {
        controlConfigLocked = true;
        // Update UI setpoint values
        updateSetpoint(controlTempSP);
        
        controlConfigLocked = false;
        newSetpointReceived = false;
    }
    lv_timer_handler();
}

void dsiplay_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map) {
    uint16_t * buf16 = (uint16_t *)px_map;

    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors(buf16, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(display);
}

void ctp_read_cb(lv_indev_t * drv, lv_indev_data_t * data) {
    uint16_t touchX = 0, touchY = 0;

    if (!ctp.touched()) {
        data->state = LV_INDEV_STATE_RELEASED;
    } else {  
        TS_Point p = ctp.getPoint();
    
        touchX = p.x;
        touchY = p.y;
    
        data->state = LV_INDEV_STATE_PRESSED;
    
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

static uint32_t lv_tick_cb(void) { return millis(); }
