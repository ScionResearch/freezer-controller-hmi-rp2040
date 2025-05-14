#pragma once

#include "sys_init.h"

// Timing defines (production)
#define COMPRESSOR_ON_TIME_MAX_ms       1800000      // 30 minutes
#define COMPRESSOR_ON_TIME_MIN_ms       150000       // 2.5 minutes
#define COMPRESSOR_OFF_TIME_MIN_ms      300000       // 5 minutes

// Hysteresis defines
#define COMPRESSOR_HYSTERESIS_DEG_C     0.5f
#define EXPECTED_DELTA_T_OVER_MAX_RUN_TIME  5.0f    // Minimum expected temperature change over maximum run time (for alarm detection)

#define DEFAULT_FAN_SPEED               50

// Control configuration filename and default modbus port
#define DEFAULT_MODBUS_TCP_PORT 502



// Function declarations
void init_control(void);
void handle_control(void);
void control_alarm_check(void);
void setTemperatureSetpoint(float newSetpoint);
void setFanSpeed(uint8_t speed);

// Global variables
extern float controlTempSP;
extern float compressorOnHysteresis;
extern float compressorOffHysteresis;
extern volatile bool compressorRunning;
extern volatile bool controlAlarm;
extern volatile bool newSetpointReceived;
extern uint32_t compressorStartTS;
extern uint32_t compressorStopTS;