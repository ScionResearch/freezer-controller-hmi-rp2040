#pragma once

#include "../sys_init.h"

// LittleFS configuration
#define CONFIG_FILENAME "/network_config.json"
#define CONFIG_MAGIC_NUMBER 0x55

// Timing defines
#define NTP_MIN_SYNC_INTERVAL 70000   // Too frequent NTP requests will cause failed connections - 70s minimum
#define NTP_UPDATE_INTERVAL 86400000  // 1 day = 86400000ms
//#define NTP_UPDATE_INTERVAL 100000 // 100 seconds (for testing!)

// NTP status defines
#define NTP_STATUS_CURRENT 0
#define NTP_STATUS_STALE 1
#define NTP_STATUS_FAILED 2

bool init_network(void);
bool handle_network(void);

bool setupEthernet(void);
bool applyNetworkConfig(void);

// NTP functions
void handleNTPUpdates(bool forceUpdate);
void ntpUpdate(void);

// Network management functions
void manageEthernet(void);

extern Wiznet5500lwIP eth;

extern char deviceMacAddress[18];

// NTP update
extern uint32_t ntpUpdateTimestamp;
extern uint32_t lastNTPUpdateTime; // Last successful NTP update time

extern bool ethernetConnected;