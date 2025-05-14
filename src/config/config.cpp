#include "config.h"

ControlConfig controlConfig;
NetworkConfig networkConfig;
ResetInfo resetInfo;

volatile bool controlConfigToSave = false;
volatile bool networkConfigToSave = false;

volatile bool controlConfigLoaded = false;
volatile bool networkConfigLoaded = false;

volatile bool controlConfigLocked = false;
volatile bool networkConfigLocked = false;

volatile bool newControlConfig = false;
volatile bool newNetworkConfig = false;

bool init_config(void) {
    if (!loadControlConfig()) {
        if (debug) Serial.printf("Failed to load control config\n");
        return false;
    }
    if (debug) Serial.printf("Control configuration loaded from file system successfully\n");
    controlConfigLoaded = true;
    
    // Load network configuration
    if (!loadNetworkConfig()) {
        // Set default configuration if load fails
        if (debug) Serial.printf("Invalid network configuration, using defaults\n");
        networkConfig.ntpEnabled = false;
        networkConfig.useDHCP = true;
        networkConfig.ip = IPAddress(192, 168, 1, 100);
        networkConfig.subnet = IPAddress(255, 255, 255, 0);
        networkConfig.gateway = IPAddress(192, 168, 1, 1);
        networkConfig.dns = IPAddress(8, 8, 8, 8);
        strcpy(networkConfig.timezone, "+12:00");
        strcpy(networkConfig.hostname, "freezer-controller");
        strcpy(networkConfig.ntpServer, "pool.ntp.org");
        networkConfig.dstEnabled = false;
        saveNetworkConfig();
    }
    if (debug) Serial.printf("Network configuration loaded from file system successfully\n");
    networkConfigLoaded = true;

    // Load reset info
    if (!loadResetInfo()) {
        // Set default configuration if load fails
        if (debug) Serial.printf("Invalid reset info, using defaults\n");
    } else {
        if (debug) Serial.printf("Reset info loaded from file system successfully\n");
        saveResetInfo(true);
    }

    return true;
}

void handle_config() {
    if (controlConfigToSave && !controlConfigLocked) {
        controlConfigLocked = true;
        saveControlConfig();
        controlConfigLocked = false;
        controlConfigToSave = false;
    }
    if (networkConfigToSave && !networkConfigLocked) {
        networkConfigLocked = true;
        saveNetworkConfig();
        networkConfigLocked = false;
        networkConfigToSave = false;
    }
}

// Control config load and save
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
    controlConfig.compressorOnHysteresis = doc["compressor_on_hysteresis"] | COMPRESSOR_HYSTERESIS_DEG_C;
    controlConfig.compressorOffHysteresis = doc["compressor_off_hysteresis"] | COMPRESSOR_HYSTERESIS_DEG_C;
    controlConfig.fanSpeed = doc["fan_speed"] | DEFAULT_FAN_SPEED;
    controlConfig.modbusTcpPort = doc["modbus_tcp_port"] | DEFAULT_MODBUS_TCP_PORT;
    
    if (debug) Serial.printf("Loaded control config: temp_sp=%.1f, on hyst=%f, off hyst=%f, fan speed=%d, modbus_port=%d\n", 
                controlConfig.temperatureSetpoint, controlConfig.compressorOnHysteresis, controlConfig.compressorOffHysteresis, controlConfig.fanSpeed, controlConfig.modbusTcpPort);
    
    return true;
}

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
    doc["compressor_on_hysteresis"] = controlConfig.compressorOnHysteresis;
    doc["compressor_off_hysteresis"] = controlConfig.compressorOffHysteresis;
    doc["fan_speed"] = controlConfig.fanSpeed;
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
    newControlConfig = true;
}

// Network config load and save
bool loadNetworkConfig()
{
  if (debug) Serial.printf("Loading network configuration:\n");
  
  // Check if LittleFS is mounted
  if (!LittleFS.begin()) {
    if (debug) Serial.printf("Failed to mount LittleFS\n");
    return false;
  }

  // Check if config file exists
  if (!LittleFS.exists(CONFIG_FILENAME)) {
    if (debug) Serial.printf("Config file not found\n");
    
    return false;
  }

  // Open config file
  File configFile = LittleFS.open(CONFIG_FILENAME, "r");
  if (!configFile) {
    if (debug) Serial.printf("Failed to open config file\n");
    
    return false;
  }

  // Allocate a buffer to store contents of the file
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, configFile);
  configFile.close();

  if (error) {
    if (debug) Serial.printf("Failed to parse config file: %s\n", error.c_str());
    
    return false;
  }

  // Check magic number
  uint8_t magicNumber = doc["magic_number"] | 0;
  if (debug) Serial.printf("Magic number: %x\n", magicNumber);
  if (magicNumber != CONFIG_MAGIC_NUMBER) {
    if (debug) Serial.printf("Invalid magic number\n");
    
    return false;
  }

  // Parse network configuration
  networkConfig.useDHCP = doc["use_dhcp"] | true;
  
  // Parse IP addresses
  IPAddress ip, subnet, gateway, dns;
  if (ip.fromString(doc["ip"] | "192.168.1.100")) networkConfig.ip = ip;
  if (subnet.fromString(doc["subnet"] | "255.255.255.0")) networkConfig.subnet = subnet;
  if (gateway.fromString(doc["gateway"] | "192.168.1.1")) networkConfig.gateway = gateway;
  if (dns.fromString(doc["dns"] | "8.8.8.8")) networkConfig.dns = dns;
  
  // Parse strings
  strlcpy(networkConfig.hostname, doc["hostname"] | "freezer-control", sizeof(networkConfig.hostname));
  strlcpy(networkConfig.ntpServer, doc["ntp_server"] | "pool.ntp.org", sizeof(networkConfig.ntpServer));
  strlcpy(networkConfig.timezone, doc["timezone"] | "+12:00", sizeof(networkConfig.timezone));
  
  // Parse booleans
  networkConfig.ntpEnabled = doc["ntp_enabled"] | false;
  networkConfig.dstEnabled = doc["dst_enabled"] | false;
  
  return true;
}

void saveNetworkConfig()
{
  if (debug) Serial.printf("Saving network configuration:\n");
  printNetConfig(networkConfig);
  
  // Check if LittleFS is mounted
  if (!LittleFS.begin()) {
    if (debug) Serial.printf("Failed to mount LittleFS\n");
    return;
  }

  // Create JSON document
  StaticJsonDocument<512> doc;
  
  // Store magic number
  doc["magic_number"] = CONFIG_MAGIC_NUMBER;
  
  // Store network configuration
  doc["use_dhcp"] = networkConfig.useDHCP;
  
  // Store IP addresses as strings
  doc["ip"] = networkConfig.ip.toString();
  doc["subnet"] = networkConfig.subnet.toString();
  doc["gateway"] = networkConfig.gateway.toString();
  doc["dns"] = networkConfig.dns.toString();
  
  // Store strings
  doc["hostname"] = networkConfig.hostname;
  doc["ntp_server"] = networkConfig.ntpServer;
  doc["timezone"] = networkConfig.timezone;
  
  // Store booleans
  doc["ntp_enabled"] = networkConfig.ntpEnabled;
  doc["dst_enabled"] = networkConfig.dstEnabled;
  
  
  // Open file for writing
  File configFile = LittleFS.open(CONFIG_FILENAME, "w");
  if (!configFile) {
    if (debug) Serial.printf("Failed to open config file for writing\n");
    
    return;
  }
  
  // Write to file
  if (serializeJson(doc, configFile) == 0) {
    if (debug) Serial.printf("Failed to write config file\n");
  }
  
  // Close file
  configFile.close();
  newNetworkConfig = true;
}

// Reset info load and save
bool loadResetInfo(void)
{
    if (!LittleFS.begin()) {
        if (debug) Serial.printf("Failed to mount LittleFS\n");
        return false;
    }
    
    // Check if the config file exists
    if (!LittleFS.exists(RESET_INFO_FILENAME)) {
        if (debug) Serial.printf("Reset info file not found, using defaults\n");
        saveResetInfo(false); // Save defaults
        return false;
    }
    
    // Open the file for reading
    File resetInfoFile = LittleFS.open(RESET_INFO_FILENAME, "r");
    if (!resetInfoFile) {
        if (debug) Serial.printf("Failed to open reset info file for reading\n");
        return false;
    }
    
    // Parse JSON content
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, resetInfoFile);
    resetInfoFile.close();
    
    if (error) {
        if (debug) Serial.printf("Failed to parse reset info JSON: %s\n", error.c_str());
        return false;
    }
    
    // Extract configuration values
    resetInfo.resetReason = doc["reset_reason"] | 0;
    resetInfo.resetCount = doc["reset_count"] | 0;
    resetInfo.wdtResetCount = doc["wdt_reset_count"] | 0;
    resetInfo.resetDateTime.year = doc["reset_datetime_year"] | 0;
    resetInfo.resetDateTime.month = doc["reset_datetime_month"] | 0;
    resetInfo.resetDateTime.day = doc["reset_datetime_day"] | 0;
    resetInfo.resetDateTime.hour = doc["reset_datetime_hour"] | 0;
    resetInfo.resetDateTime.minute = doc["reset_datetime_minute"] | 0;
    resetInfo.resetDateTime.second = doc["reset_datetime_second"] | 0;
    
    if (debug) {
        const char* resetReasonTxt[8] = {
            "UNKNOWN RESET",
            "PWRON RESET",
            "RUN PIN RESET",
            "SOFT RESET",
            "WDT RESET",
            "DEBUG RESET",
            "GLITCH RESET",
            "BROWNOUT RESET"
        };

        Serial.printf("Loaded reset info:\n");
        Serial.printf("%t- Last reset reason: %s\n", resetReasonTxt[resetInfo.resetReason]);
        Serial.printf("%t- Reset count: %d\n", resetInfo.resetCount);
        Serial.printf("%t- WDT reset count: %d\n", resetInfo.wdtResetCount);
        Serial.printf("%t- Last reset timestamp: %d-%d-%d %d:%d:%d\n", resetInfo.resetDateTime.year, resetInfo.resetDateTime.month, resetInfo.resetDateTime.day, resetInfo.resetDateTime.hour, resetInfo.resetDateTime.minute, resetInfo.resetDateTime.second);
    }
    return true;
}

void setResetInfo(void)
{
    resetInfo.resetReason = rp2040.getResetReason();
    resetInfo.resetCount +=1;
    if (resetInfo.resetReason == 4) resetInfo.wdtResetCount +=1;
    getGlobalDateTime(resetInfo.resetDateTime);
}

void saveResetInfo(bool updateResetInfo)
{
    if (debug) Serial.printf("Saving reset info...\n");
    
    // Check if LittleFS is mounted
    if (!LittleFS.begin()) {
        if (debug) Serial.printf("Failed to mount LittleFS\n");
        return;
    }

    if (updateResetInfo) setResetInfo();

    // Save to modbus TCP registers
    inputRegisterBuffer[24] = resetInfo.resetReason;
    memcpy(&inputRegisterBuffer[25], &resetInfo.resetCount, sizeof(uint32_t));
    memcpy(&inputRegisterBuffer[27], &resetInfo.wdtResetCount, sizeof(uint32_t));
    inputRegisterBuffer[29] = resetInfo.resetDateTime.year;
    inputRegisterBuffer[30] = resetInfo.resetDateTime.month;
    inputRegisterBuffer[31] = resetInfo.resetDateTime.day;
    inputRegisterBuffer[32] = resetInfo.resetDateTime.hour;
    inputRegisterBuffer[33] = resetInfo.resetDateTime.minute;
    inputRegisterBuffer[34] = resetInfo.resetDateTime.second;
    
    // Create JSON document
    StaticJsonDocument<1024> doc;
    
    // Store configuration values
    doc["reset_reason"] = resetInfo.resetReason;
    doc["reset_count"] = resetInfo.resetCount;
    doc["wdt_reset_count"] = resetInfo.wdtResetCount;
    doc["reset_datetime_year"] = resetInfo.resetDateTime.year;
    doc["reset_datetime_month"] = resetInfo.resetDateTime.month;
    doc["reset_datetime_day"] = resetInfo.resetDateTime.day;
    doc["reset_datetime_hour"] = resetInfo.resetDateTime.hour;
    doc["reset_datetime_minute"] = resetInfo.resetDateTime.minute;
    doc["reset_datetime_second"] = resetInfo.resetDateTime.second;
    
    // Open file for writing
    File resetInfoFile = LittleFS.open(RESET_INFO_FILENAME, "w");
    if (!resetInfoFile) {
        if (debug) Serial.printf("Failed to open reset info file for writing\n");
        return;
    }
    
    // Write to file
    if (serializeJson(doc, resetInfoFile) == 0) {
        if (debug) Serial.printf("Failed to write reset info file\n");
    } else {
        if (debug) Serial.printf("Reset info saved successfully\n");
    }

    if (debug) {
        const char* resetReasonTxt[8] = {
            "UNKNOWN RESET",
            "PWRON RESET",
            "RUN PIN RESET",
            "SOFT RESET",
            "WDT RESET",
            "DEBUG RESET",
            "GLITCH RESET",
            "BROWNOUT RESET"
        };

        Serial.printf("Saved new reset info:\n");
        Serial.printf("%t- Last reset reason: %s\n", resetReasonTxt[resetInfo.resetReason]);
        Serial.printf("%t- Reset count: %d\n", resetInfo.resetCount);
        Serial.printf("%t- WDT reset count: %d\n", resetInfo.wdtResetCount);
        Serial.printf("%t- Last reset timestamp: %d-%d-%d %d:%d:%d\n", resetInfo.resetDateTime.year, resetInfo.resetDateTime.month, resetInfo.resetDateTime.day, resetInfo.resetDateTime.hour, resetInfo.resetDateTime.minute, resetInfo.resetDateTime.second);
    }
    
    // Close file
    resetInfoFile.close();
}

// Debug functions --------------------------------------------------------->
void printNetConfig(NetworkConfig config)
{
  if (debug) Serial.printf("Mode: %s\n", config.useDHCP ? "DHCP" : "Static");
  if (config.useDHCP) {
    if (debug) Serial.printf("IP: %s\n", eth.localIP().toString().c_str());
    if (debug) Serial.printf("Subnet: %s\n", eth.subnetMask().toString().c_str());
    if (debug) Serial.printf("Gateway: %s\n", eth.gatewayIP().toString().c_str());
    if (debug) Serial.printf("DNS: %s\n", eth.dnsIP().toString().c_str());
  } else {
    if (debug) Serial.printf("IP: %s\n", config.ip.toString().c_str());
    if (debug) Serial.printf("Subnet: %s\n", config.subnet.toString().c_str());
    if (debug) Serial.printf("Gateway: %s\n", config.gateway.toString().c_str());
    if (debug) Serial.printf("DNS: %s\n", config.dns.toString().c_str());
  }
  if (debug) Serial.printf("Timezone: %s\n", config.timezone);
  if (debug) Serial.printf("Hostname: %s\n", config.hostname);
  if (debug) Serial.printf("NTP Server: %s\n", config.ntpServer);
  if (debug) Serial.printf("NTP Enabled: %s\n", config.ntpEnabled ? "true" : "false");
  if (debug) Serial.printf("DST Enabled: %s\n", config.dstEnabled ? "true" : "false");
}