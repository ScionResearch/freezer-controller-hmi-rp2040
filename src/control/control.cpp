#include "control.h"

// Global variables
float controlTempSP = 0.0f;
float compressorOnHysteresis = 0.5f;
float compressorOffHysteresis = 0.5f;
volatile bool controlAlarm = false;
volatile bool compressorRunning = false;
uint32_t compressorStartTS = 0;
uint32_t compressorStopTS = 0 - COMPRESSOR_OFF_TIME_MIN_ms;
uint32_t lastSetpointChangeTime = 0;
volatile bool setpointNeedsSaving = false;
volatile bool newSetpointReceived = false;

void init_control(void) {
    pinMode(PIN_COMPRESSOR_EN, OUTPUT);
    digitalWrite(PIN_COMPRESSOR_EN, LOW);
    
    // Load control configuration
    if (!controlConfigLoaded) {
        if (debug) Serial.println("ERROR: Control configuration not loaded, init config first!");
        return;
    }
    while (controlConfigLocked) delay(100);
    controlConfigLocked = true;
    controlTempSP = controlConfig.temperatureSetpoint;
    compressorOnHysteresis = controlConfig.compressorOnHysteresis;
    compressorOffHysteresis = controlConfig.compressorOffHysteresis;
    newSetpointReceived = true;
    controlConfigLocked = false;
}

void handle_control(void) {
    if (newControlConfig && !controlConfigLocked) {
        controlConfigLocked = true;
        controlTempSP = controlConfig.temperatureSetpoint;
        compressorOnHysteresis = controlConfig.compressorOnHysteresis;
        compressorOffHysteresis = controlConfig.compressorOffHysteresis;
        controlConfigLocked = false;
        newControlConfig = false;
        updateSetpoint(controlTempSP);
    }
    // Check if we need to save the setpoint after a change
    if (setpointNeedsSaving && (millis() - lastSetpointChangeTime > 10000)) {
        setpointNeedsSaving = false;
        controlConfigToSave = true;
    }
    
    if (sensorLocked) return;
    sensorLocked = true;
    if (compressorRunning) {
        if ((millis() - compressorStartTS) < COMPRESSOR_ON_TIME_MIN_ms) {
            sensorLocked = false;
            return;
        }
        
        bool stop = false;
        if (sensor.temperature < (controlTempSP - controlConfig.compressorOffHysteresis)) stop = true;
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
        
        if (sensor.temperature > (controlTempSP + controlConfig.compressorOnHysteresis)) {
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

// Call this function when the setpoint is changed by the UI
void setTemperatureSetpoint(float newSetpoint) {
    if (controlConfigLocked) return;
    controlConfigLocked = true;
    controlTempSP = newSetpoint;
    controlConfig.temperatureSetpoint = newSetpoint;
    controlConfigLocked = false;
    lastSetpointChangeTime = millis();
    setpointNeedsSaving = true;
}