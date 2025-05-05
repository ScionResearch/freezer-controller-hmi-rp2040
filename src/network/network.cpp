#include "network.h"

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
bool init_network() {
    return setupEthernet();
}

bool handle_network(void) {
    manageEthernet();
    if (networkConfig.ntpEnabled) handleNTPUpdates(false);
    return ethernetConnected;
}

bool setupEthernet()
{
  if (!networkConfigLoaded) {
    if (debug) Serial.printf("ERROR: Network configuration not loaded, init config first!\n");
    return false;
  }

  SPI.setMOSI(PIN_ETH_MOSI);
  SPI.setMISO(PIN_ETH_MISO);
  SPI.setSCK(PIN_ETH_SCK);
  SPI.setCS(PIN_ETH_CS);

  eth.setSPISpeed(50000000);

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
  uint32_t timeout = 5000;
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
  return ethernetConnected;
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
