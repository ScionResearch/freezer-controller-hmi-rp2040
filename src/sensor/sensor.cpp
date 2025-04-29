#include "sensor.h"

ModbusRTUMaster bus(Serial2);

sensorData_t sensor;
bool sensorLocked = false;
bool sensorAlarm = false;
uint16_t holdingRegisters[100];

bool sensorInit(void) {
    Serial.println("Starting Modbus interface to sensor...");
    Serial2.setTX(PIN_RS485_TX);
    Serial2.setRX(PIN_RS485_RX);
    bus.begin(MODBUS_BAUD);
    return manageSensor();
}

bool manageSensor(void) {
    if (!bus.readHoldingRegisters(SENSOR_SLAVE_ID, 0, holdingRegisters, 22)) {
        sensorAlarm = true;
        setAlarmState();
        return false;
    }
    while (sensorLocked) {
        delay(1);
    }
    sensorLocked = true;
    memcpy(&sensor, holdingRegisters, sizeof(sensor));
    sensorLocked = false;
    sensorAlarm = false;
    setAlarmState();
    return true;
}