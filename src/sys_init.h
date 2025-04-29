#pragma once

// Hardware pins:
#define PIN_RTC_SDA    0
#define PIN_RTC_SCL    1
#define PIN_TFT_SDA    2
#define PIN_TFT_SCL    3
#define PIN_TFT_DC     4
#define PIN_TFT_RST    5
#define PIN_TFT_D0     6
#define PIN_TFT_D1     7
#define PIN_TFT_D2     8
#define PIN_TFT_D3     9
#define PIN_TFT_D4     10
#define PIN_TFT_D5     11
#define PIN_TFT_D6     12
#define PIN_TFT_D7     13
#define PIN_ETH_IRQ    14
#define PIN_ETH_RST    15
#define PIN_ETH_MISO   16
#define PIN_ETH_CS     17
#define PIN_ETH_SCK    18
#define PIN_ETH_MOSI   19
#define PIN_RS485_TX   20
#define PIN_RS485_RX   21
#define PIN_TFT_WR     22
#define PIN_TFT_INT    23
#define PIN_TFT_BL     24
#define PIN_RUN_LED    25
#define PIN_V_PSU      26
#define PIN_V_5V       27
#define PIN_COMPRESSOR_EN    28
#define PIN_AUX    29

// Lib includes:
#include <Arduino.h>
#include <W5500lwIP.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Adafruit_FT6206.h>
#include "MCP79410.h"
#include "ModbusRTUMaster.h"

// Header includes:
#include "network/network.h"
#include "web/web.h"
#include "time/rtc_manager.h"
#include "display/display.h"
#include "ui/screenMain.h"
#include "sensor/sensor.h"
#include "control/control.h"
