# Project Structure

## Directory Organization

```
dummy_ap/
├── components/              # Reusable ESP-IDF components
│   ├── led/                # LED control component
│   │   ├── include/        # Public LED API headers
│   │   ├── led.c           # LED implementation with FreeRTOS task
│   │   ├── CMakeLists.txt  # Component build configuration
│   │   └── Kconfig         # Component configuration options
│   └── protocol_examples_common/  # WiFi/Ethernet connectivity utilities
│       ├── include/        # Protocol utility headers
│       ├── connect.c       # Connection management
│       ├── wifi_connect.c  # WiFi-specific connection logic
│       ├── protocol_examples_utils.c  # Common protocol utilities
│       ├── CMakeLists.txt  # Component build configuration
│       ├── Kconfig.projbuild  # Project-level configuration
│       └── sdkconfig.defaults  # Default SDK configuration
├── main/                   # Main application entry point
│   ├── main.c              # HTTP server and application logic
│   ├── CMakeLists.txt      # Main component build configuration
│   └── Kconfig.projbuild   # Application configuration (async settings)
├── ref/                    # Reference examples
│   └── ledc_fade_example_main.c  # LEDC PWM fade reference code
├── .devcontainer/          # VS Code dev container configuration
│   ├── devcontainer.json   # Container settings
│   └── Dockerfile          # Container image definition
├── CMakeLists.txt          # Root project build configuration
├── sdkconfig               # ESP-IDF SDK configuration
└── dependencies.lock       # Component dependency lock file
```

## Core Components and Relationships

### 1. Main Application (main/)
- **Entry Point**: app_main() function initializes system and starts HTTP server
- **HTTP Server**: Implements async request handler pattern with worker thread pool
- **Request Routing**: Three endpoints - "/" (index), "/long" (async), "/quick" (sync)
- **Dependencies**: Uses protocol_examples_common for connectivity, led component for visual feedback

### 2. LED Component (components/led/)
- **Purpose**: Provides LED control abstraction with multiple display modes
- **Architecture**: FreeRTOS task-based with notification mechanism for mode changes
- **Modes**: OFF, ON, BLINK_SLOW, BLINK_FAST, BREATH
- **Integration**: Runs as independent task, controlled via led_set_mode() API

### 3. Protocol Examples Common (components/protocol_examples_common/)
- **Purpose**: Abstracts WiFi/Ethernet connection setup
- **Key Function**: example_connect() handles network initialization
- **Configuration**: Kconfig-driven network settings (SSID, password, etc.)
- **Portability**: Supports both WiFi and Ethernet based on menuconfig selection

## Architectural Patterns

### Asynchronous Request Handler Pattern
```
Client Request → Queue Check → Worker Available?
                                ├─ Yes → Queue Request → Worker Processes
                                └─ No  → Return 503 Busy
```

**Key Components**:
- **Request Queue**: FreeRTOS queue holding pending async requests
- **Worker Pool**: Fixed number of worker tasks (CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS)
- **Counting Semaphore**: Tracks available workers in real-time
- **Request Copying**: httpd_req_async_handler_begin() creates owned request copy

### Event-Driven Server Lifecycle
```
Network Event → Event Handler → Server Action
├─ IP_EVENT_STA_GOT_IP → connect_handler() → Start Server
└─ WIFI_EVENT_STA_DISCONNECTED → disconnect_handler() → Stop Server
```

### FreeRTOS Task Architecture
- **Worker Tasks**: Multiple async request processors (priority 5)
- **LED Task**: Independent LED control loop (priority 5)
- **HTTP Server**: Managed by ESP-IDF httpd component
- **Synchronization**: Semaphores, queues, and task notifications

## Build System
- **ESP-IDF CMake**: Standard ESP-IDF project structure
- **Minimal Build**: Configured with MINIMAL_BUILD to reduce binary size
- **Component Dependencies**: Automatic dependency resolution via CMakeLists.txt
- **Kconfig Integration**: Compile-time configuration through menuconfig
