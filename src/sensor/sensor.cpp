#include "sensor.h"

ModbusRTUMaster bus(Serial2);

sensorData_t sensor;
volatile bool sensorLocked = false;
volatile bool sensorAlarm = false;
uint16_t holdingRegisters[100];

bool sensorInit(void) {
    Serial2.setTX(PIN_RS485_TX);
    Serial2.setRX(PIN_RS485_RX);
    Serial2.setFIFOSize(128);
    bus.begin(MODBUS_BAUD);
    return bus.readHoldingRegisters(SENSOR_SLAVE_ID, 0, holdingRegisters, 1);
}

bool manageSensor(void) {
    bool success = bus.readHoldingRegisters(SENSOR_SLAVE_ID, 0, holdingRegisters, 22);
    if (!success) {
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