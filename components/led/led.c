#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/ledc_types.h"

#define NOTIFY_LED_MODE_CHANGE (1 << 0)

static const char *TAG = "led";

#define LEDC_TIMER CONFIG_LED_TIMER_ID
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_GPIO CONFIG_LED_GPIO

#if CONFIG_LED_OUTPUT_INVERT
#define LEDC_OUTPUT_INVERT (1)
#else
#define LEDC_OUTPUT_INVERT (0)
#endif

#define LEDC_DUTY_RESOLUTION LEDC_TIMER_10_BIT
#define LEDC_DUTY_MAX ((1UL << LEDC_DUTY_RESOLUTION) - 1)
#define BLINK_SLOW_MS CONFIG_LED_BLINK_SLOW_MS
#define BLINK_FAST_MS CONFIG_LED_BLINK_FAST_MS
#define BREATH_FADE_MS CONFIG_LED_BREATH_FADE_MS

static enum LED_MODE led_status = LED_MODE_OFF;
static TaskHandle_t led_task_handle = NULL;

enum LED_MODE led_get_mode(void) {
    return led_status;
}

void led_set_mode(enum LED_MODE mode) {
    ESP_LOGI(TAG, "Setting LED mode to %d", mode);
    led_status = mode;
    if (led_task_handle != NULL) {
        xTaskNotify(led_task_handle, NOTIFY_LED_MODE_CHANGE, eSetBits);
        ESP_LOGI(TAG, "Notified task of mode change to %d", mode);
    }
}

static void led_display_off(void) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    xTaskNotifyWait(0, NOTIFY_LED_MODE_CHANGE, NULL, portMAX_DELAY);
}

static void led_display_on(void) {
    ESP_LOGD(TAG, "LED on");
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_MAX);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    xTaskNotifyWait(0, NOTIFY_LED_MODE_CHANGE, NULL, portMAX_DELAY);
}

static void led_display_blink_slow(void) {
    ESP_LOGD(TAG, "Blink slow cycle");
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_MAX);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(BLINK_SLOW_MS));
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(BLINK_SLOW_MS));
}

static void led_display_blink_fast(void) {
    ESP_LOGD(TAG, "Blink fast cycle");
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_MAX);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(BLINK_FAST_MS));
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(BLINK_FAST_MS));
}

static void led_display_breath(void) {
    ESP_LOGD(TAG, "Breath cycle");
    ledc_set_fade_with_time(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_MAX, BREATH_FADE_MS);
    ledc_fade_start(LEDC_MODE, LEDC_CHANNEL, LEDC_FADE_WAIT_DONE);
    ledc_set_fade_with_time(LEDC_MODE, LEDC_CHANNEL, 0, BREATH_FADE_MS);
    ledc_fade_start(LEDC_MODE, LEDC_CHANNEL,LEDC_FADE_WAIT_DONE);
}

static void led_ledc_init(void)
{
    ledc_timer_config_t timer = {
        .duty_resolution = LEDC_DUTY_RESOLUTION,
        .freq_hz = 4000,
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .channel = LEDC_CHANNEL,
        .duty = 0,
        .gpio_num = LEDC_GPIO,
        .speed_mode = LEDC_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER,
        .flags.output_invert = LEDC_OUTPUT_INVERT,
    };
    ledc_channel_config(&channel);

    ledc_fade_func_install(0);
}

void led_init(void) {
    ESP_LOGI(TAG, "Initializing LED on GPIO %d", LEDC_GPIO);
    led_ledc_init();
    led_set_mode(LED_MODE_OFF);
    xTaskNotifyWait(0, NOTIFY_LED_MODE_CHANGE, NULL, 0);
    led_task_handle = xTaskGetCurrentTaskHandle();
    ESP_LOGI(TAG, "LED initialized");
}

void led_main_task(void* arg)
{
    ESP_LOGI(TAG, "LED task started");
    led_init();
    while (1) {
        switch (led_status) {
            case LED_MODE_OFF:
                led_display_off();
                break;
            case LED_MODE_ON:
                led_display_on();
                break;
            case LED_MODE_BLINK_SLOW:
                led_display_blink_slow();
                break;
            case LED_MODE_BLINK_FAST:
                led_display_blink_fast();
                break;
            case LED_MODE_BREATH:
                led_display_breath();
                break;
        }
    }
}
