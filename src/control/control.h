#pragma once

#include "sys_init.h"


// Timing defines (test)
#define COMPRESSOR_ON_TIME_MAX_ms       30000      // 30 seconds
#define COMPRESSOR_ON_TIME_MIN_ms       3000       // 3 seconds
#define COMPRESSOR_OFF_TIME_MIN_ms      5000       // 5 seconds

// Timing defines (production)
/*#define COMPRESSOR_ON_TIME_MAX_ms       1800000      // 30 minutes
#define COMPRESSOR_ON_TIME_MIN_ms       180000       // 3 minutes
#define COMPRESSOR_OFF_TIME_MIN_ms      300000       // 5 minutes*/

// Hysteresis defines
#define COMPRESSOR_HYSTERESIS_DEG_C     0.5f
#define EXPECTED_DELTA_T_OVER_MAX_RUN_TIME  5.0f    // Minimum expected temperature change over maximum run time (for alarm detection)

// Control configuration filename and default modbus port
#define CONTROL_CONFIG_FILENAME "/control_config.json"
#define DEFAULT_MODBUS_TCP_PORT 502

// Control configuration structure
struct ControlConfig {
    float temperatureSetpoint;
    uint16_t modbusTcpPort;
};

// Function declarations
void init_control(void);
void manage_control(void);
void control_alarm_check(void);
bool loadControlConfig(void);
void saveControlConfig(void);
void setTemperatureSetpoint(float newSetpoint);

// Global variables
extern float controlTempSP;
extern bool compressorRunning;
extern bool controlAlarm;
extern uint32_t compressorStartTS;
extern uint32_t compressorStopTS;
extern ControlConfig controlConfig;