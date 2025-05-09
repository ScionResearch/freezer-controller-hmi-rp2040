# Freezer Controller HMI (RP2040)

A dual-core freezer control system with touch screen interface, web configuration, and environmental monitoring built on the Raspberry Pi Pico (RP2040) platform.

## Overview

This project implements a Human-Machine Interface (HMI) for freezer control systems. It monitors temperature, humidity, and pressure using ModbusRTU sensors and controls a freezer compressor based on configurable setpoints with appropriate safety timings. The system features both a local touchscreen display and remote web access for monitoring and configuration.

## Hardware

- **Microcontroller**: Raspberry Pi Pico (RP2040) dual-core ARM Cortex-M0+
- **Display**: ILI9488 TFT display with FT6206 capacitive touch controller
- **Network**: W5500 Ethernet controller
- **Timekeeping**: MCP79410 RTC module
- **Communications**: RS485 ModbusRTU for sensor interfacing
- **Outputs**: Compressor control relay
- **Storage**: LittleFS for configuration persistence

## Features

- **Dual-core operation**:
  - Core 0: Network services, web server
  - Core 1: Display, sensor communication, control logic
- **Real-time environmental monitoring**:
  - Temperature, humidity, and pressure sensing
  - Dew point calculation
  - Compressor control with safety timing constraints
- **Networking**:
  - Configurable static/DHCP network settings
  - Web-based configuration interface
  - NTP time synchronization 
- **User Interface**:
  - Local touchscreen interface
  - Web-based remote monitoring and control
- **Configuration Persistence**:
  - LittleFS-based storage using JSON format
  - Safe for multi-core applications

## Software Architecture

The system is built on Arduino framework for RP2040 and uses:
- LittleFS for configuration storage (replaced EEPROM for improved multi-core safety)
- ArduinoJson for configuration serialization
- LVGL for touchscreen GUI
- TFT_eSPI for display driver
- ModbusRTU for sensor communication
- WebServer for remote interface

## Configuration

Network and system settings are stored in a JSON configuration file (`/network_config.json`) on the LittleFS filesystem. Default settings are applied if the configuration file is not found.

## Safety Features

- Minimum compressor on/off times to prevent rapid cycling
- Maximum run time limits for compressor protection
- Temperature delta monitoring to detect refrigeration failures
- Humidity and pressure monitoring
- LittleFS for configuration storage prevents flash access conflicts in multi-core operation

## Building and Flashing

This project uses PlatformIO for building and deployment. The configuration includes:
- 256KB filesystem allocation for LittleFS
- CPU clock at 250MHz
- Custom USB descriptors

To build and flash:
1. Open the project in PlatformIO
2. Connect the RP2040 board
3. Upload the firmware and filesystem