# Product Overview

## Project Purpose
ESP32 Dummy Access Point - An embedded HTTP server application demonstrating asynchronous request handling on ESP32 microcontrollers. This project showcases how to build responsive web servers that can handle multiple long-running HTTP requests concurrently while maintaining responsiveness for quick synchronous requests.

## Key Features
- **Asynchronous HTTP Request Handling**: Implements worker thread pool pattern to process long-running HTTP requests without blocking the server
- **Concurrent Request Processing**: Supports multiple simultaneous async requests (configurable via Kconfig)
- **WiFi Connectivity**: Integrates ESP-IDF protocol examples for WiFi/Ethernet connection management
- **LED Control System**: Custom LED component with multiple display modes (on, off, blink slow/fast, breathing effect)
- **Resource Management**: Implements semaphore-based worker availability tracking and request queuing
- **Event-Driven Architecture**: Automatic server start/stop based on network connection events

## Target Users and Use Cases

### Target Users
- Embedded systems developers working with ESP32 microcontrollers
- IoT application developers building web-enabled devices
- Engineers learning asynchronous programming patterns on resource-constrained devices
- Developers prototyping ESP32-based web servers

### Use Cases
- **IoT Device Control Panels**: Web interface for controlling and monitoring embedded devices
- **Long-Running Task Management**: Handling time-consuming operations (sensor data collection, calculations) via HTTP without blocking other requests
- **Educational Examples**: Learning resource for ESP-IDF async HTTP server patterns
- **Prototype Development**: Foundation for building responsive web servers on ESP32 hardware
- **Status Monitoring Systems**: Real-time device status reporting with LED visual feedback
