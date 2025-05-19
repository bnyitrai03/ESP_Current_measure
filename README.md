# ESP Current Monitoring System

This project implements a current monitoring system using an ESP32-S3 microcontroller and an INA219 current sensor. The system reads current measurements from the sensor and uploads the data to a server via HTTP POST requests.

## Overview

The system continuously reads current values from the INA219 sensor, timestamps each reading, and sends the data to a configured server endpoint. The project uses the ESP-IDF framework and is organized using a component-based architecture.

## Components

### Hardware Components
- ESP32-S3 microcontroller
- INA219 current sensor (connected via I2C)
- Breadboard
- Jumpers

### Software Components
1. **IN219** - Handles communication with the INA219 current sensor
2. **Communication** - Contains modules for:
   - WiFi connectivity
   - HTTP client for data posting
   - I2C communication management
3. **Main Application** - Coordinates tasks and data flow

## Features

- WiFi connectivity with automatic reconnection
- NTP time synchronization
- I2C communication for sensor reading
- HTTP POST for data transmission
- Multi-tasking architecture using FreeRTOS tasks
- JSON data formatting using ArduinoJson library

## Configuration

Before building the project, you need to create a `secret.h` file in the `main/include` directory with the following content:

```cpp
#pragma once

#define SSID "your_wifi_ssid"
#define PASSWORD "your_wifi_password"
#define SERVER_URL "http://your_server_ip:5000/data"
```

## Building and Flashing

This project uses the ESP-IDF build system. To build and flash:

1. Set up ESP-IDF environment
2. Navigate to project directory
3. Run `idf.py build`
4. Run `idf.py -p [PORT] flash monitor`

## Server

The project includes a simple Flask server (`server.py`) that receives the sensor data and saves it to a CSV file:

- The server listens on port 5000
- Data is received via POST requests to the `/data` endpoint
- Data is stored in a CSV file named `current_data.csv`

To run the server:
```bash
pip install flask
python server.py
```

## Dependencies

- ESP-IDF (Espressif IoT Development Framework)
- ArduinoJson library (version 7.3.0 or later)
- Flask (for the server component)
