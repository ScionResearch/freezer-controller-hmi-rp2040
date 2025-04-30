#pragma once

#include "sys_init.h"

#define MAX_MODBUS_CLIENTS 4
#define NUM_MODBUS_INPUT_REGISTERS 30

// Client management
struct ModbusClientConnection {
    WiFiClient client;
    ModbusTCPServer server;
    bool connected;
    IPAddress clientIP;
    unsigned long connectionTime;
};

extern WiFiServer modbusServer;

extern ModbusClientConnection modbusClients[MAX_MODBUS_CLIENTS];
extern int connectedClients;
extern uint16_t inputRegisterBuffer[NUM_MODBUS_INPUT_REGISTERS];
extern bool regBufLocked;
extern bool newValues;

void init_modbusTCP(void);

void handle_modbusTCP(void);