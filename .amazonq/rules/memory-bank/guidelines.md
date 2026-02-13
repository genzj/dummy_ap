# Development Guidelines

## Agentic Coding Guidelines

- **No Building Attepmts**: Avoid executing building automatically. Instead, rely on syntax analysis, code reference and LLM analysis of existing code and references and documents to write correct code.
- **Use References**: Check the `/ref` folder for relevant code and components. They are of the latest IDF offical examples. Consider implementing the required features by taking and adjusting code snippets from them.
- **Ask Questions**: Ask users to provide more contexts or references (by adding files into the `/ref` folder) if uncertain about current task or its implementation.

## Code Quality Standards

### File Headers and Licensing
- **License Headers**: All source files include license information at the top (Public Domain, CC0, BSD-2-Clause, or Unlicense)
- **SPDX Tags**: Use SPDX-FileCopyrightText and SPDX-License-Identifier tags for clear licensing
- **Copyright Attribution**: Include copyright notices and contributor information where applicable
- **Example Comments**: Mark example code with disclaimer about "AS IS" distribution without warranties

### Code Formatting and Structure
- **Indentation**: 4 spaces for indentation (no tabs)
- **Brace Style**: Opening braces on same line for functions and control structures
- **Line Length**: Keep lines reasonable; break long parameter lists across multiple lines
- **Whitespace**: Single blank line between functions; no trailing whitespace
- **Alignment**: Align struct member initializers and multi-line assignments for readability
- **Configurable**: Feature related configurations in a certain module, for example sleeping duration, GPIO, etc., should be customizable to component users. Adding these values to the KConfig with proper default value and documentation for these purposes.

### Naming Conventions
- **Functions**: snake_case for all function names (e.g., `led_set_mode`, `example_connect`, `start_webserver`)
- **Variables**: snake_case for local and global variables (e.g., `worker_ready_count`, `request_queue`)
- **Constants/Macros**: UPPER_SNAKE_CASE for preprocessor defines (e.g., `LED_MODE_CHANGE_NOTIFY`, `ASYNC_WORKER_TASK_PRIORITY`)
- **Enums**: UPPER_SNAKE_CASE for enum values with descriptive prefixes (e.g., `LED_MODE_OFF`, `LED_MODE_BLINK_SLOW`)
- **Types**: snake_case with _t suffix for typedefs (e.g., `httpd_req_handler_t`, `httpd_async_req_t`)
- **Static Variables**: Prefix with `static` keyword; use descriptive names (e.g., `static const char *TAG`)

### Documentation Standards
- **Function Comments**: Use Doxygen-style comments with @brief, @param, @return, @note tags
- **Block Comments**: Multi-line comments for complex logic explanation
- **Inline Comments**: Single-line comments for clarification of non-obvious code
- **Header Guards**: Use `#pragma once` instead of traditional include guards (consistent across all headers)
- **API Documentation**: Public APIs must have complete documentation with parameter descriptions and usage notes

## Semantic Patterns and Idioms

### ESP-IDF Initialization Pattern
```c
// Standard ESP-IDF initialization sequence (5/5 files follow this)
ESP_ERROR_CHECK(nvs_flash_init());
ESP_ERROR_CHECK(esp_netif_init());
ESP_ERROR_CHECK(esp_event_loop_create_default());
ESP_ERROR_CHECK(example_connect());
```
**Usage**: Always initialize in this order - NVS, network interface, event loop, then connectivity

### Error Handling Pattern
```c
// Check return values with ESP_ERROR_CHECK macro (4/5 files)
esp_err_t err = httpd_req_async_handler_begin(req, &copy);
if (err != ESP_OK) {
    return err;
}

// Alternative: Direct ESP_ERROR_CHECK for critical operations
ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
```
**Usage**: Use ESP_ERROR_CHECK for initialization; manual checks for runtime operations

### Logging Pattern
```c
// ESP-IDF logging with severity levels (5/5 files use this)
static const char *TAG = "example";  // Define TAG at file scope

ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
ESP_LOGE(TAG, "Failed to allocate memory for headers");
ESP_LOGW(TAG, "worker stopped");
```
**Usage**: Always define TAG constant; use appropriate log levels (LOGI, LOGE, LOGW)

### FreeRTOS Task Creation Pattern
```c
// Standard task creation with error checking (3/5 files)
bool success = xTaskCreate(
    worker_task,                    // Task function
    "async_req_worker",             // Task name (for debugging)
    ASYNC_WORKER_TASK_STACK_SIZE,   // Stack size
    (void *)0,                      // Task parameter
    ASYNC_WORKER_TASK_PRIORITY,     // Priority
    &worker_handles[i]              // Task handle (or NULL)
);

if (!success) {
    ESP_LOGE(TAG, "Failed to start asyncReqWorker");
    continue;
}
```
**Usage**: Always check task creation success; use descriptive task names; store handles when needed

### FreeRTOS Synchronization Patterns

#### Counting Semaphore for Resource Tracking
```c
// Create counting semaphore to track available workers (2/5 files)
worker_ready_count = xSemaphoreCreateCounting(
    CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS,  // Max count
    0                                    // Initial count
);

// Worker signals availability
xSemaphoreGive(worker_ready_count);

// Check availability before queuing
if (xSemaphoreTake(worker_ready_count, ticks) == false) {
    ESP_LOGE(TAG, "No workers are available");
    return ESP_FAIL;
}
```

#### Task Notification for Event Signaling
```c
// Lightweight notification mechanism (2/5 files)
#define LED_MODE_CHANGE_NOTIFY (1 << 0)

// Sender: Notify task of mode change
xTaskNotify(led_task_handle, LED_MODE_CHANGE_NOTIFY, eSetBits);

// Receiver: Wait for notification
xTaskNotifyWait(0, LED_MODE_CHANGE_NOTIFY, NULL, portMAX_DELAY);
```

#### Queue for Work Distribution
```c
// Create queue for async requests (2/5 files)
request_queue = xQueueCreate(1, sizeof(httpd_async_req_t));

// Send to queue with timeout
if (xQueueSend(request_queue, &async_req, pdMS_TO_TICKS(100)) == false) {
    ESP_LOGE(TAG, "worker queue is full");
    return ESP_FAIL;
}

// Receive from queue (blocking)
if (xQueueReceive(request_queue, &async_req, portMAX_DELAY)) {
    // Process request
}
```

### Memory Management Pattern
```c
// Dynamic allocation with NULL checks (3/5 files)
buf = malloc(buf_len);
if (buf == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for headers");
    return ESP_FAIL;
}
// ... use buffer ...
free(buf);  // Always free when done
```
**Usage**: Always check malloc return value; free memory in all code paths

### Configuration-Driven Constants
```c
// Use Kconfig values for compile-time configuration (5/5 files)
#define ASYNC_WORKER_TASK_STACK_SIZE CONFIG_EXAMPLE_ASYNC_WORKER_TASK_STACK_SIZE
config.max_open_sockets = CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS + 1;

// Conditional compilation based on target
#if CONFIG_IDF_TARGET_ESP32
    // ESP32-specific code
#else
    // Other targets
#endif
```
**Usage**: Prefer CONFIG_ macros over hardcoded values; support multiple targets

### Static Function Organization
```c
// Private implementation functions marked static (5/5 files)
static void worker_task(void *p) { /* ... */ }
static esp_err_t long_async(httpd_req_t *req) { /* ... */ }
static void led_display_on(void) { /* ... */ }

// Public API functions without static
void led_init(void) { /* ... */ }
esp_err_t example_connect(void) { /* ... */ }
```
**Usage**: Mark internal functions static; only expose necessary APIs in headers

### Struct Initialization Pattern
```c
// Designated initializers for struct configuration (4/5 files)
httpd_config_t config = HTTPD_DEFAULT_CONFIG();
config.lru_purge_enable = true;
config.max_open_sockets = CONFIG_EXAMPLE_MAX_ASYNC_REQUESTS + 1;

// Or full designated initialization
const httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
};
```
**Usage**: Use designated initializers for clarity; start with default configs when available

### Infinite Loop Task Pattern
```c
// Standard FreeRTOS task loop (3/5 files)
void worker_task(void *p) {
    ESP_LOGI(TAG, "starting async req task worker");
    
    while (true) {
        // Wait for work
        // Process work
        // Signal completion
    }
    
    // Cleanup (rarely reached)
    ESP_LOGW(TAG, "worker stopped");
    vTaskDelete(NULL);
}
```
**Usage**: Tasks run infinite loops; include cleanup code after loop; delete self on exit

### Event Handler Registration Pattern
```c
// Register event handlers with context (2/5 files)
ESP_ERROR_CHECK(esp_event_handler_register(
    IP_EVENT,                    // Event base
    IP_EVENT_STA_GOT_IP,        // Event ID
    &connect_handler,            // Handler function
    &server                      // User context
));

// Handler signature
static void connect_handler(
    void* arg,                   // User context
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data
) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    // Handle event
}
```

### C Linkage for C++ Compatibility
```c
// All public headers use C linkage guards (3/3 headers)
#ifdef __cplusplus
extern "C" {
#endif

// API declarations

#ifdef __cplusplus
}
#endif
```
**Usage**: Always wrap public header APIs with extern "C" for C++ compatibility

### Pointer Parameter Validation
```c
// Validate pointer parameters before use (2/5 files)
if (!src || !dest) {
    return 0;  // or return early
}
```
**Usage**: Check pointer parameters at function entry; return appropriate error codes

### ISR-Safe Callback Pattern
```c
// ISR callback with FreeRTOS primitives (1/5 files - reference example)
static IRAM_ATTR bool cb_ledc_fade_end_event(const ledc_cb_param_t *param, void *user_arg) {
    BaseType_t taskAwoken = pdFALSE;
    
    if (param->event == LEDC_FADE_END_EVT) {
        SemaphoreHandle_t counting_sem = (SemaphoreHandle_t) user_arg;
        xSemaphoreGiveFromISR(counting_sem, &taskAwoken);
    }
    
    return (taskAwoken == pdTRUE);
}
```
**Usage**: Mark ISR callbacks with IRAM_ATTR; use FromISR variants; track task wakeup

## Component Architecture Patterns

### Component Public API Design
- **Header Location**: Public APIs in `include/` directory
- **Implementation**: Private implementation in component root
- **Minimal Exposure**: Only expose necessary functions; keep internals static
- **Clear Separation**: Separate interface (header) from implementation (source)

### Component Configuration
- **Kconfig Files**: Use `Kconfig` for component-specific options
- **Kconfig.projbuild**: Use for project-level configuration that appears in main menu
- **Defaults**: Provide sensible defaults in Kconfig definitions
- **Documentation**: Include help text for all configuration options

### Component Dependencies
- **CMakeLists.txt**: Declare dependencies explicitly via REQUIRES
- **Minimal Dependencies**: Only depend on what's actually needed
- **Component Isolation**: Design components to be reusable across projects

## Best Practices Summary

1. **Always use ESP_ERROR_CHECK** for initialization code
2. **Define TAG constant** at file scope for logging
3. **Check malloc return values** before using allocated memory
4. **Use designated initializers** for struct configuration
5. **Mark internal functions static** to limit scope
6. **Validate pointer parameters** at function entry
7. **Use FreeRTOS primitives correctly**: queues for data, semaphores for counting, notifications for events
8. **Include license headers** in all source files
9. **Document public APIs** with Doxygen-style comments
10. **Use CONFIG_ macros** instead of hardcoded constants
11. **Support multiple targets** with conditional compilation
12. **Wrap headers with extern "C"** for C++ compatibility
13. **Use #pragma once** for header guards
14. **Follow snake_case naming** for functions and variables
15. **Use UPPER_SNAKE_CASE** for macros and enum values
