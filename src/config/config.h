#pragma once

#include "sys_init.h"

#define CONTROL_CONFIG_FILENAME "/control_config.json"
#define NETWORK_CONFIG_FILENAME "/network_config.json"
#define RESET_INFO_FILENAME "/reset_info.json"

// Control configuration structure
struct ControlConfig {
    float temperatureSetpoint;
    float compressorOnHysteresis;
    float compressorOffHysteresis;
    uint16_t modbusTcpPort;
};

// Network configuration structure
struct NetworkConfig
{
    IPAddress ip;
    IPAddress subnet;
    IPAddress gateway;
    IPAddress dns;
    bool useDHCP;
    char hostname[32];
    char ntpServer[64];
    bool ntpEnabled;
    char timezone[8]; // Format: "+13:00"
    bool dstEnabled;  // Daylight Saving Time enabled
};

struct ResetInfo {
    uint8_t resetReason = 0;
    uint32_t resetCount = 0;
    uint32_t wdtResetCount = 0;
    DateTime resetDateTime;
};

// Function declarations
bool init_config(void);
void handle_config(void);

// Controller configuration
bool loadControlConfig(void);
void saveControlConfig(void);

// Network configuration
bool loadNetworkConfig(void);
void saveNetworkConfig(void);

// Reset info
bool loadResetInfo(void);
void setResetInfo(void);
void saveResetInfo(bool updateResetInfo);

// Debug functions
void printNetConfig(NetworkConfig config);

// Global variables
extern ControlConfig controlConfig;
extern NetworkConfig networkConfig;
extern ResetInfo resetInfo;

extern volatile bool controlConfigToSave;
extern volatile bool networkConfigToSave;

extern volatile bool controlConfigLoaded;
extern volatile bool networkConfigLoaded;

extern volatile bool newControlConfig;
extern volatile bool newNetworkConfig;

extern volatile bool controlConfigLocked;
extern volatile bool networkConfigLocked;