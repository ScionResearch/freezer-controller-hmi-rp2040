#include "sys_init.h"

// Global variables
volatile bool core0setupComplete = false;
volatile bool core1setupComplete = false;

static uint16_t display_buf[(320 * 480)/10];

void init_ui(void);

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

uint32_t rtcLoopTS;
uint32_t sensorLoopTS;

volatile bool debug = true;

void setup() {
    if (debug) Serial.begin(115200);
    uint32_t startTime = millis();

    while (!Serial) {
        delay(10);
        if (millis() - startTime > 5000) {
            debug = false;
            break; // Timeout after 5 seconds
        }
    }
    if (debug) Serial.println("Core 0 starting...");
    // Ethernet interface on core 0 ---------->
    init_rtc();
    if (debug) Serial.println("RTC initialised");

    init_network();
    if (debug) Serial.println("Network initialised");

    init_webserver();
    if (debug) Serial.println("Webserver initialised");

    init_modbusTCP();
    if (debug) Serial.println("Modbus TCP initialised");

    core0setupComplete = true;
    if (debug) Serial.println("Core 0 startup complete, waiting for core 1 to start...");

    while (!core1setupComplete) {
        delay(1);
    }

    rtcLoopTS = millis();
}

void setup1() {
    // Everything else on core 1 ------------->   
    
    while (!core0setupComplete) {
        delay(100);
    }

    if (debug) Serial.println("Core 1 starting...");
    
    sensorInit();
    if (debug) Serial.println("Sensor initialised");   

    init_control();
    if (debug) Serial.println("Control initialised");

    init_ui();
    if (debug) Serial.println("UI initialised");

    core1setupComplete = true;
    if (debug) Serial.println("Core 1 startup complete, entering main loop...");

    sensorLoopTS = millis();
}

void loop() {
    manageNetwork();
    handle_modbusTCP();

    // 1 second loop
    if (millis() - rtcLoopTS > 1000) {
        rtcLoopTS += 1000;
        manageTime();
    }
}


void loop1() {
    lv_timer_handler();

    // 2 second loop
    if (millis() - sensorLoopTS > 2000) {
        sensorLoopTS += 2000;
        if (!manageSensor()) {
            if (debug) Serial.println("Error reading sensor data");
            setIconLink(false);
            setIconBell(true);
        } else if (!sensorLocked) {
            sensorLocked = true;
            sensor.temperature -= 30;   // Testing only!
            if (debug) Serial.printf("Temp: %0.2f °C| Humid: %0.2f %%RH | DP: %0.2f °C | Pres: %0.2f hPa\n", sensor.temperature, sensor.humidity, sensor.dewPoint, sensor.pressure);
            updateProcessValues(sensor.temperature, sensor.humidity, sensor.pressure);

            // Update the modbus TCP register buffer
            if (!regBufLocked) {
                regBufLocked = true;
                memcpy(inputRegisterBuffer, &sensor, sizeof(sensor));
                memcpy(&inputRegisterBuffer[22], &controlTempSP, sizeof(float));
                regBufLocked = false;
                newValues = true;
            }
            sensorLocked = false;
            
            setIconLink(true);
            setIconBell(false);
            manage_control();
        }
    }
}


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