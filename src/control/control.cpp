#include "control.h"

// Global variables
float controlTempSP = 0.0f;
bool controlAlarm = false;
bool compressorRunning = false;
uint32_t compressorStartTS = 0;
uint32_t compressorStopTS = 0;

void init_control(void) {
    pinMode(PIN_COMPRESSOR_EN, OUTPUT);
    digitalWrite(PIN_COMPRESSOR_EN, LOW);
}

void manage_control(void) {
    if (sensorLocked) return;
    sensorLocked = true;
    if (compressorRunning) {
        if ((millis() - compressorStartTS) < COMPRESSOR_ON_TIME_MIN_ms) {
            sensorLocked = false;
            return;
        }
        
        bool stop = false;
        if (sensor.temperature < (controlTempSP - COMPRESSOR_HYSTERESIS_DEG_C)) stop = true;
        else if ((millis() - compressorStartTS) > COMPRESSOR_ON_TIME_MAX_ms) {
            control_alarm_check();  // Compressor timed out, check if we are cooling effectively
            stop = true;
        }
        if (stop) {
            compressorRunning = false;
            compressorStopTS = millis();
            digitalWrite(PIN_COMPRESSOR_EN, LOW);
            setIconCycle(false);
        }
    } else {    // Compressor stopped
        if (millis() - compressorStopTS < COMPRESSOR_OFF_TIME_MIN_ms) {
            sensorLocked = false;
            return;
        }
        
        if (sensor.temperature > (controlTempSP + COMPRESSOR_HYSTERESIS_DEG_C)) {
            compressorRunning = true;
            compressorStartTS = millis();
            digitalWrite(PIN_COMPRESSOR_EN, HIGH);
            setIconCycle(true);
        }
    }
    sensorLocked = false;
}

void control_alarm_check(void) {
    static float prevTemp = 0.0f;
    float deltaT = prevTemp - sensor.temperature;
    prevTemp = sensor.temperature;
    if (deltaT < EXPECTED_DELTA_T_OVER_MAX_RUN_TIME) {
        controlAlarm = true;
    } else controlAlarm = false;
    setAlarmState();
}
