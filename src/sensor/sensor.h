#pragma once

#include "sys_init.h"

#define MODBUS_BAUD 500000
#define SENSOR_SLAVE_ID 1

struct sensorData_t {
    uint32_t tempSensorID_1;
    uint32_t tempSensorID_2;
    uint32_t pressSensorID;
    uint32_t status;
    uint32_t txcount;
    float temperature;
    float humidity;
    float dewPoint;
    float pressure;
    float deltaT;
    float deltaH;
};

bool sensorInit(void);
bool manageSensor(void);

extern bool sensorLocked;
extern sensorData_t sensor;
extern bool sensorAlarm;