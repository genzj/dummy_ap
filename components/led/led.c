#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define NOTIFY_LED_MODE_CHANGE (1 << 0)

#define LEDC_TIMER CONFIG_LED_TIMER_ID
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_GPIO CONFIG_LED_GPIO
#define LEDC_DUTY_RESOLUTION LEDC_TIMER_10_BIT
#define LEDC_DUTY_MAX ((2UL << LEDC_DUTY_RESOLUTION) - 1)
#define BLINK_SLOW_MS CONFIG_LED_BLINK_SLOW_MS
#define BLINK_FAST_MS CONFIG_LED_BLINK_FAST_MS
#define BREATH_FADE_MS CONFIG_LED_BREATH_FADE_MS

static enum LED_MODE led_status = LED_MODE_OFF;
static TaskHandle_t led_task_handle = NULL;
static SemaphoreHandle_t fade_sem = NULL;

enum LED_MODE led_get_mode(void) {
    return led_status;
}

void led_set_mode(enum LED_MODE mode) {
    led_status = mode;
    if (led_task_handle != NULL) {
        xTaskNotify(led_task_handle, NOTIFY_LED_MODE_CHANGE, eSetBits);
    }
}

static IRAM_ATTR bool led_fade_cb(const ledc_cb_param_t *param, void *user_arg) {
    BaseType_t taskAwoken = pdFALSE;
    if (param->event == LEDC_FADE_END_EVT) {
        xSemaphoreGiveFromISR(fade_sem, &taskAwoken);
    }
    return (taskAwoken == pdTRUE);
}

static void led_display_off(void) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    xTaskNotifyWait(0, NOTIFY_LED_MODE_CHANGE, NULL, portMAX_DELAY);
}

static void led_display_on(void) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_MAX);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    xTaskNotifyWait(0, NOTIFY_LED_MODE_CHANGE, NULL, portMAX_DELAY);
}

static void led_display_blink_slow(void) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_MAX);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(BLINK_SLOW_MS));
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(BLINK_SLOW_MS));
}

static void led_display_blink_fast(void) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_MAX);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(BLINK_FAST_MS));
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(BLINK_FAST_MS));
}

static void led_display_breath(void) {
    ledc_set_fade_with_time(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY_MAX, BREATH_FADE_MS);
    ledc_fade_start(LEDC_MODE, LEDC_CHANNEL, LEDC_FADE_NO_WAIT);
    xSemaphoreTake(fade_sem, portMAX_DELAY);
    ledc_set_fade_with_time(LEDC_MODE, LEDC_CHANNEL, 0, BREATH_FADE_MS);
    ledc_fade_start(LEDC_MODE, LEDC_CHANNEL, LEDC_FADE_NO_WAIT);
    xSemaphoreTake(fade_sem, portMAX_DELAY);
}

static void led_ledc_init(void)
{
    ledc_timer_config_t timer = {
        .duty_resolution = LEDC_DUTY_RESOLUTION,
        .freq_hz = 5000,
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
        .flags.output_invert = 0
    };
    ledc_channel_config(&channel);

    ledc_fade_func_install(0);

    fade_sem = xSemaphoreCreateBinary();
    ledc_cbs_t callbacks = {
        .fade_cb = led_fade_cb
    };
    ledc_cb_register(LEDC_MODE, LEDC_CHANNEL, &callbacks, NULL);
}

void led_init(void) {
    led_ledc_init();
    led_set_mode(LED_MODE_OFF);
    xTaskNotifyWait(0, NOTIFY_LED_MODE_CHANGE, NULL, 0);
    led_task_handle = xTaskGetCurrentTaskHandle();
}

void led_main_task(void* arg)
{
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
