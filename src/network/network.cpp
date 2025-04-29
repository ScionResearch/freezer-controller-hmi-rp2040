#include "network.h"

// Global variables
NetworkConfig networkConfig;

Wiznet5500lwIP eth(PIN_ETH_CS, SPI, PIN_ETH_IRQ);

// NTP update tracking
bool ntpUpdateRequested = true;
uint32_t ntpUpdateTimestamp = 0 - NTP_MIN_SYNC_INTERVAL;
uint32_t lastNTPUpdateTime = 0; // Last successful NTP update time

// Device MAC address (stored as string)
char deviceMacAddress[18];

bool ethernetConnected = false;
unsigned long lastNetworkCheckTime = 0;

// Network component initialisation functions ------------------------------>
void init_network() {
    setupEthernet();
}

void manageNetwork(void) {
    manageEthernet();
    if (networkConfig.ntpEnabled) handleNTPUpdates(false);
}

void setupEthernet()
{
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
  } else if (debug) Serial.printf("Network configuration loaded from file system successfully\n");

  SPI.setMOSI(PIN_ETH_MOSI);
  SPI.setMISO(PIN_ETH_MISO);
  SPI.setSCK(PIN_ETH_SCK);
  SPI.setCS(PIN_ETH_CS);

  eth.setSPISpeed(80000000);
  //lwipPollingPeriod(10);

  eth.hostname(networkConfig.hostname);

  if (debug) Serial.printf("Applying network configuration\n");

  // Apply network configuration
  if (!applyNetworkConfig()) {
    if (debug) Serial.printf("Failed to apply network configuration\n");
  } else {
    // Get and store MAC address
    uint8_t mac[6];
    eth.macAddress(mac);
    snprintf(deviceMacAddress, sizeof(deviceMacAddress), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    if (debug) Serial.printf("MAC Address: %s\n", deviceMacAddress);
  }

  // Wait for Ethernet to connect
  uint32_t startTime = millis();
  uint32_t timeout = 10000;
  while (eth.linkStatus() == LinkOFF) {
    if (millis() - startTime > timeout) {
      break;
    }
  }

  if (eth.linkStatus() == LinkOFF) {
    if (debug) Serial.printf("Ethernet not connected\n");
    ethernetConnected = false;
  }
  else {
    if (debug) Serial.println("Getting IP address...");
    startTime = millis();
    while (!eth.connected()) {
      if (millis() - startTime > timeout) {
        break;
      }
    }
    if (debug) Serial.printf("Ethernet connected, IP address: %s, Gateway: %s\n",
                eth.localIP().toString().c_str(),
                eth.gatewayIP().toString().c_str());
    ethernetConnected = true;
  }
}

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
}

bool applyNetworkConfig()
{
  if (networkConfig.useDHCP)
  {
    // Call eth.end() to release the DHCP lease if we already had one since last boot (handles changing networks on the fly)
    // NOTE: requires modification of end function in LwipIntDev.h, added dhcp_release_and_stop(&_netif); before netif_remove(&_netif);)
    eth.end();
    if (debug) Serial.flush();
    if (!eth.begin()) {
      if (debug) Serial.printf("Failed to configure Ethernet using DHCP, falling back to 192.168.1.100\n");
      IPAddress defaultIP = {192, 168, 1, 100};
      eth.config(defaultIP);
      if (!eth.begin()) return false;
    } //else if (debug) Serial.printf("Successfully configured Ethernet using DHCP\n");
  }
  else
  {
    eth.config(networkConfig.ip, networkConfig.gateway, networkConfig.subnet, networkConfig.dns);
    if (!eth.begin()) return false;
  }
  return true;
}

// Network management functions --------------------------------------------->
// Handle ethernet plug and unplug events (from main loop)
void manageEthernet(void)
{
  static bool waitingForIP = false;
  // Do network tasks if ethernet is connected
  if (ethernetConnected) {
    if (eth.linkStatus() == LinkOFF) {
      ethernetConnected = false;
      if (debug) Serial.printf("Ethernet disconnected, waiting for reconnect\n");
    } else if (!eth.connected()) {
      if (!waitingForIP) {
        waitingForIP = true;
        if (debug) Serial.printf("Waiting for IP address...\n");
      }
    } else if (waitingForIP) {
      waitingForIP = false;
      if (debug) Serial.printf("Connected, IP address: %s, Gateway: %s\n",
                  eth.localIP().toString().c_str(),
                  eth.gatewayIP().toString().c_str());
    } else {
      // Ethernet is still connected
      handleWebServer();
      handleNTPUpdates(false);
    }
  }
  else if (eth.linkStatus() == LinkON) {
    ethernetConnected = true;
    if(!applyNetworkConfig()) {
      if (debug) Serial.printf("Failed to apply network configuration!\n");
    }
    else {
      if (debug) Serial.printf("Ethernet re-connected\n");
    }
  }
}

// NTP management functions ------------------------------------------------>
void ntpUpdate(void) {
  static WiFiUDP udp;
  static NTPClient timeClient(udp, networkConfig.ntpServer);
  static bool clientInitialized = false;

  
  if (!clientInitialized)
  {
    timeClient.begin();
    clientInitialized = true;
  }

  if (!eth.linkStatus()) return;

  if (!timeClient.update()) {
    if (debug) Serial.printf("Failed to get time from NTP server, retrying\n");
    bool updateSuccessful = false;
    for (int i = 0; i < 3; i++) {
      if (timeClient.update()) {
        updateSuccessful = true;
        break;
      }
      delay(10);
   }
    if (!updateSuccessful) {
      if (debug) Serial.printf("Failed to get time from NTP server, giving up\n");
      return;
    }
  }
  // Get NTP time
  time_t epochTime = timeClient.getEpochTime();

  // Apply timezone offset
  int tzHours = 0, tzMinutes = 0, tzDSToffset = 0;
  if (networkConfig.dstEnabled) {
    tzDSToffset = 3600;
  }
  sscanf(networkConfig.timezone, "%d:%d", &tzHours, &tzMinutes);
  epochTime += (tzHours * 3600 + tzMinutes * 60 + tzDSToffset);

  // Convert to DateTime and update using thread-safe function
  DateTime newTime = epochToDateTime(epochTime);
  if (!updateGlobalDateTime(newTime))
  {
    if (debug) Serial.printf("Failed to update time from NTP\n");
  }
  else
  {
    if (debug) Serial.printf("Time updated from NTP server\n");
    lastNTPUpdateTime = millis(); // Record the time of successful update
  }
}

void handleNTPUpdates(bool forceUpdate) {
  if (!networkConfig.ntpEnabled) return;
  uint32_t timeSinceLastUpdate = millis() - ntpUpdateTimestamp;

  // Check if there's an NTP update request or if it's time for a scheduled update
  if (ntpUpdateRequested || timeSinceLastUpdate > NTP_UPDATE_INTERVAL || forceUpdate)
  {
    if (timeSinceLastUpdate < NTP_MIN_SYNC_INTERVAL) {
      if (debug) Serial.printf("Time since last NTP update: %ds - skipping\n", timeSinceLastUpdate/1000);
      return;
    }
    ntpUpdate();
    ntpUpdateTimestamp = millis();
    ntpUpdateRequested = false; // Clear the request flag
  }
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