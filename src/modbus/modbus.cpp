#include "modbus.h"

WiFiServer modbusServer;

ModbusClientConnection modbusClients[MAX_MODBUS_CLIENTS];
int connectedClients;
uint16_t inputRegisterBuffer[NUM_MODBUS_INPUT_REGISTERS];
bool regBufLocked = false;
bool newValues = false;

void init_modbusTCP(void) {
    modbusServer.begin(controlConfig.modbusTcpPort);
    
    if (debug) Serial.print("Starting Modbus server on port: ");
    if (debug) Serial.println(controlConfig.modbusTcpPort);
    
    // Initialize all ModbusTCPServer instances
    for (int i = 0; i < MAX_MODBUS_CLIENTS; i++) {
        modbusClients[i].connected = false;
        
        // Initialize each ModbusTCPServer with the same unit ID (1)
        if (!modbusClients[i].server.begin(1)) {
            if (debug) Serial.print("Failed to start Modbus TCP Server for client ");
            if (debug) Serial.println(i);
            continue;
        }
        
        // Configure Modbus registers for each client server
        modbusClients[i].server.configureInputRegisters(0x00, NUM_MODBUS_INPUT_REGISTERS);
    }
    
    if (debug) Serial.println("Modbus TCP Server started");
}

void handle_modbusTCP(void) {
    // Check for new client connections
    WiFiClient newClient = modbusServer.accept();
    
    if (newClient) {
        // Find an available slot for the new client
        bool clientAdded = false;
        for (int i = 0; i < MAX_MODBUS_CLIENTS; i++) {
            if (!modbusClients[i].connected) {
                if (debug) Serial.print("New client connected to slot ");
                if (debug) Serial.println(i);
                
                // Store the client and mark as connected
                modbusClients[i].client = newClient;
                modbusClients[i].connected = true;
                modbusClients[i].clientIP = newClient.remoteIP();
                modbusClients[i].connectionTime = millis();
                
                // Accept the connection on this server instance
                modbusClients[i].server.accept(modbusClients[i].client);
                if (debug) Serial.println("Modbus server accepted client connection");
                
                // Initialise the input register states with the master copy
                while (regBufLocked) delay(1);
                regBufLocked = true;
                for (int j = 0; j < NUM_MODBUS_INPUT_REGISTERS; j++) {
                    modbusClients[i].server.inputRegisterWrite(j, inputRegisterBuffer[j]);
                }
                regBufLocked = false;
                
                connectedClients++;
                clientAdded = true;
                break;
            }
        }
        
        if (!clientAdded) {
            if (debug) Serial.println("No available slots for new client");
            newClient.stop();
        }
    }
    
    bool addedNewValues = true;
    
    // Poll all connected clients
    for (int i = 0; i < MAX_MODBUS_CLIENTS; i++) {
        if (modbusClients[i].connected) {
            if (modbusClients[i].client.connected()) {
                // Poll this client's Modbus server
                if (modbusClients[i].server.poll()) {
                    if (debug) Serial.println("Modbus server recieved new request");
                }
                // Update input registers for this specific client
                if (!regBufLocked && newValues) {
                    regBufLocked = true;
                    for (int j = 0; j < NUM_MODBUS_INPUT_REGISTERS; j++) {
                        modbusClients[i].server.inputRegisterWrite(j, inputRegisterBuffer[j]);
                    }
                    regBufLocked = false;
                    
                } else addedNewValues = false;
            } else {
                // Client disconnected
                if (debug) Serial.print("Client disconnected from slot ");
                if (debug) Serial.println(i);
                modbusClients[i].connected = false;
                modbusClients[i].client.stop();
                connectedClients--;
            }
        }
    }

    if (newValues && addedNewValues) newValues = false;
}