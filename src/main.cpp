#include "sys_init.h"

// Global variables
bool core0setupComplete = false;
bool core1setupComplete = false;

/*static uint16_t display_buf[(320 * 480)/10];

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

static uint32_t lv_tick_cb(void) { return millis(); }*/

uint32_t loopTS;
bool serialReady = false;

void setup() {
    // Serial for debug
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Starting freezer control system...");
    serialReady = true;

    // Ethernet interface on core 0 ---------->
    init_network();
    init_webserver();

    core0setupComplete = true;
}

void setup1() {
    // Everything else on core 1 ------------->
    while (!serialReady) {
        ;
    }
    init_rtc();
    

    // Init LVGL
    /*lv_init();
    //lv_log_register_print_cb(log_print_cb);
    lv_display_t * display = lv_display_create(320, 480);
    Serial.println("Display created and LVGL initialised");

    displayInit();

    // Register display and input device drivers
    lv_display_set_flush_cb(display, dsiplay_flush_cb);
    Serial.println("Flush callback registered");

    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, ctp_read_cb);
    lv_indev_set_long_press_time(indev, 2000); 
    Serial.println("CTP read callback registered");

    // Set screen draw buffers
    lv_display_set_buffers(display, display_buf, NULL, sizeof(display_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Register tick timer callback
    lv_tick_set_cb(lv_tick_cb);
    Serial.println("Tick callback registered");

    // Setup screen
    screenMainInit();

    // Start modbus interface to sensor
    if(!sensorInit()) {
        Serial.println("Error starting modbus interface to sensor");
    } else {
        Serial.println("Modbus interface to sensor started");
        Serial.printf("Temperature: %0.2f °C| Humidity: %0.2f %%RH | Pressure: %0.2f hPa\n", sensor.temperature, sensor.humidity, sensor.pressure);
    }
    init_control();*/
    core1setupComplete = true;
    while (!core0setupComplete) {
        ;
    }
    //loopTS = millis();
}

void loop() {
    manageNetwork();
}

void loop1() {
    manageTime();
    delay(100);
    /*lv_timer_handler();
    if (millis() - loopTS > 2000) {
        loopTS += 2000;
        if (!manageSensor()) {
            Serial.println("Error reading sensor data");
            setIconLink(false);
            setIconBell(true);
        } else {
            sensor.temperature -= 30;   // Testing only!
            Serial.printf("Temperature: %0.2f °C| Humidity: %0.2f %%RH | Dew Point: %0.2f °C | Pressure: %0.2f hPa\n", sensor.temperature, sensor.humidity, sensor.dewPoint, sensor.pressure);
            updateProcessValues(sensor.temperature, sensor.humidity, sensor.pressure);
            setIconLink(true);
            setIconBell(false);
            manage_control();
        }
    }*/
}