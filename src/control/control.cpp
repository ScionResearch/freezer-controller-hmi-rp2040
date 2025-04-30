#include "control.h"

// Global variables
float controlTempSP = 0.0f;
bool controlAlarm = false;
bool compressorRunning = false;
uint32_t compressorStartTS = 0;
uint32_t compressorStopTS = 0;
ControlConfig controlConfig = {
    .temperatureSetpoint = 0.0f,
    .modbusTcpPort = DEFAULT_MODBUS_TCP_PORT
};
uint32_t lastSetpointChangeTime = 0;
bool setpointNeedsSaving = false;

void init_control(void) {
    pinMode(PIN_COMPRESSOR_EN, OUTPUT);
    digitalWrite(PIN_COMPRESSOR_EN, LOW);
    
    // Load control configuration
    if (loadControlConfig()) {
        // Set the temperature setpoint from config
        controlTempSP = controlConfig.temperatureSetpoint;
    }
}

void manage_control(void) {
    // Check if we need to save the setpoint after a change
    if (setpointNeedsSaving && (millis() - lastSetpointChangeTime > 10000)) {
        setpointNeedsSaving = false;
        saveControlConfig();
    }
    
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

// Call this function when the setpoint is changed by the UI
void setTemperatureSetpoint(float newSetpoint) {
    controlTempSP = newSetpoint;
    controlConfig.temperatureSetpoint = newSetpoint;
    lastSetpointChangeTime = millis();
    setpointNeedsSaving = true;
}

// Load control configuration from LittleFS
bool loadControlConfig() {
    // Check if LittleFS is mounted
    if (!LittleFS.begin()) {
        if (debug) Serial.printf("Failed to mount LittleFS\n");
        return false;
    }
    
    // Check if the config file exists
    if (!LittleFS.exists(CONTROL_CONFIG_FILENAME)) {
        if (debug) Serial.printf("Control config file not found, using defaults\n");
        saveControlConfig(); // Save defaults
        return false;
    }
    
    // Open the file for reading
    File configFile = LittleFS.open(CONTROL_CONFIG_FILENAME, "r");
    if (!configFile) {
        if (debug) Serial.printf("Failed to open control config file for reading\n");
        return false;
    }
    
    // Parse JSON content
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    
    if (error) {
        if (debug) Serial.printf("Failed to parse control config JSON: %s\n", error.c_str());
        return false;
    }
    
    // Extract configuration values
    controlConfig.temperatureSetpoint = doc["temperature_setpoint"] | 0.0f;
    controlConfig.modbusTcpPort = doc["modbus_tcp_port"] | DEFAULT_MODBUS_TCP_PORT;
    
    if (debug) Serial.printf("Loaded control config: temp_sp=%.1f, modbus_port=%d\n", 
                controlConfig.temperatureSetpoint, controlConfig.modbusTcpPort);
    
    return true;
}

// Save control configuration to LittleFS
void saveControlConfig() {
    if (debug) Serial.printf("Saving control configuration...\n");
    
    // Check if LittleFS is mounted
    if (!LittleFS.begin()) {
        if (debug) Serial.printf("Failed to mount LittleFS\n");
        return;
    }
    
    // Create JSON document
    StaticJsonDocument<256> doc;
    
    // Store configuration values
    doc["temperature_setpoint"] = controlConfig.temperatureSetpoint;
    doc["modbus_tcp_port"] = controlConfig.modbusTcpPort;
    
    // Open file for writing
    File configFile = LittleFS.open(CONTROL_CONFIG_FILENAME, "w");
    if (!configFile) {
        if (debug) Serial.printf("Failed to open control config file for writing\n");
        return;
    }
    
    // Write to file
    if (serializeJson(doc, configFile) == 0) {
        if (debug) Serial.printf("Failed to write control config file\n");
    } else {
        if (debug) Serial.printf("Control config saved successfully\n");
    }
    
    // Close file
    configFile.close();
}
