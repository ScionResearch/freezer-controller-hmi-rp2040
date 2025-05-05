#include "sys_init.h"

// Global variables
volatile bool core0setupComplete = false;
volatile bool core1setupComplete = false;

bool core1_separate_stack = true;

uint32_t rtcLoopTS;
uint32_t sensorLoopTS;

volatile bool debug = true;

void setup() {
    if (debug) Serial.begin(115200);
    uint32_t startTime = millis();

    // WDT setup
    rp2040.wdt_begin(7000);

    while (!Serial) {
        delay(10);
        if (millis() - startTime > 5000) {
            debug = false;
            break; // Timeout after 5 seconds
        }
    }

    rp2040.wdt_reset();

    if (debug) Serial.println("Core 0 starting...");
    // Ethernet interface on core 0 ---------->
    init_rtc();
    if (debug) Serial.println("RTC initialised");

    init_config();
    if (debug) Serial.println("Config initialised");

    rp2040.wdt_reset();
    init_network();
    if (debug) Serial.println("Network initialised");

    rp2040.wdt_reset();
    init_webserver();
    if (debug) Serial.println("Webserver initialised");

    rp2040.wdt_reset();
    init_modbusTCP();
    if (debug) Serial.println("Modbus TCP initialised");

    rp2040.wdt_reset();
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
    
    rp2040.wdt_reset();
    sensorInit();
    if (debug) Serial.println("Sensor initialised");   

    rp2040.wdt_reset();
    init_control();
    if (debug) Serial.println("Control initialised");

    rp2040.wdt_reset();
    init_ui();
    if (debug) Serial.println("UI initialised");

    rp2040.wdt_reset();
    core1setupComplete = true;
    if (debug) Serial.println("Core 1 startup complete, entering main loop...");

    sensorLoopTS = millis();
}

void loop() {
    handle_network();
    handle_modbusTCP();
    handle_config();

    // 1 second loop
    if (millis() - rtcLoopTS > 1000) {
        rtcLoopTS += 1000;
        manageTime();
    }
}


void loop1() {
    handle_ui();

    // 2 second loop
    if (millis() - sensorLoopTS > 2000) {
        sensorLoopTS += 2000;
        if (!manageSensor()) {
            if (debug) Serial.println("Error reading sensor data");
            setIconLink(false);
            setIconBell(true);
        } else if (!sensorLocked) {
            sensorLocked = true;
            //sensor.temperature -= 30;   // Testing only!
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
            handle_control();
        }
        // Ethernet status check
        setIconEth(ethernetConnected);

        // Debug memory usage
        if (debug) {
            Serial.printf("Free heap: %d bytes\n", rp2040.getFreeHeap());
            Serial.printf("Used heap: %d bytes\n", rp2040.getUsedHeap());
            Serial.printf("Total heap: %d bytes\n", rp2040.getTotalHeap());
        }
        rp2040.wdt_reset();
    }
}