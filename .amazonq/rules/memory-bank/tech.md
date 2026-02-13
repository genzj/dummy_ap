# Technology Stack

## Programming Languages
- **C (C99)**: Primary language for all application and component code
- **CMake**: Build system configuration and component management

## Core Frameworks and SDKs

### ESP-IDF (Espressif IoT Development Framework)
- **Version**: Requires CMake 3.16+ (specified in root CMakeLists.txt)
- **Purpose**: Official development framework for ESP32 microcontrollers
- **Key Components Used**:
  - `esp_http_server`: HTTP server implementation with async support
  - `esp_wifi`: WiFi stack and management
  - `esp_event`: Event loop system for network events
  - `esp_netif`: Network interface abstraction
  - `nvs_flash`: Non-volatile storage for configuration
  - `esp_log`: Logging framework

### FreeRTOS
- **Integration**: Built into ESP-IDF
- **Usage**: Task management, queues, semaphores, notifications
- **Key APIs**:
  - Task creation and management (xTaskCreate, vTaskDelete)
  - Queue operations (xQueueCreate, xQueueSend, xQueueReceive)
  - Semaphores (xSemaphoreCreateCounting, xSemaphoreTake, xSemaphoreGive)
  - Task notifications (xTaskNotify, xTaskNotifyWait)

## Build System and Dependencies

### CMake Build Configuration
```cmake
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(dummy_ap)
```

### Component Structure
Each component (led, protocol_examples_common) has:
- `CMakeLists.txt`: Defines sources, includes, and dependencies
- `Kconfig` or `Kconfig.projbuild`: Configuration options
- `include/`: Public header files

### Configuration System (Kconfig)
- **EXAMPLE_MAX_ASYNC_REQUESTS**: Max simultaneous async requests (default: 2)
- **EXAMPLE_ASYNC_WORKER_TASK_STACK_SIZE**: Worker task stack size (default: 2560 bytes)
- Network configuration via protocol_examples_common component

## Development Environment

### Supported Platforms
- **Primary**: Linux, macOS, Windows (via ESP-IDF toolchain)
- **Container**: Dev container support (.devcontainer/ configuration)

### Development Tools
- **ESP-IDF Toolchain**: Compiler (xtensa-esp32-elf-gcc), flash tools, monitor
- **clangd**: Language server configuration (.clangd file present)
- **Git**: Version control with .gitignore for build artifacts

## Key Dependencies

### Runtime Dependencies
- ESP-IDF components (built-in):
  - freertos
  - esp_http_server
  - esp_wifi
  - esp_event
  - nvs_flash
  - esp_netif
  - esp_log
  - esp_system

### Component Dependencies
- **main** depends on:
  - protocol_examples_common (network connectivity)
  - led (LED control)
  - ESP-IDF HTTP server components

## Development Commands

### Build and Flash
```bash
# Configure project (menuconfig)
idf.py menuconfig

# Build project
idf.py build

# Flash to device
idf.py flash

# Monitor serial output
idf.py monitor

# Build, flash, and monitor in one command
idf.py build flash monitor
```

### Configuration
```bash
# Full configuration menu
idf.py menuconfig

# Set target chip (if needed)
idf.py set-target esp32
```

### Cleaning
```bash
# Clean build artifacts
idf.py fullclean

# Clean and rebuild
idf.py fullclean build
```

## Hardware Requirements
- **Target**: ESP32 microcontroller (any variant supported by ESP-IDF)
- **Connectivity**: WiFi or Ethernet capability
- **Optional**: LED for visual feedback (GPIO configuration in led component)
