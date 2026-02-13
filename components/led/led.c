#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_MODE_CHANGE_NOTIFY (1 << 0)

static enum LED_MODE led_status = LED_MODE_OFF;
static TaskHandle_t led_task_handle = NULL;

enum LED_MODE led_get_mode(void) {
    return led_status;
}

void led_set_mode(enum LED_MODE mode) {
    led_status = mode;
    if (led_task_handle != NULL) {
        xTaskNotify(led_task_handle, LED_MODE_CHANGE_NOTIFY, eSetBits);
    }
}

static void led_display_on(void) {
}

static void led_display_blink_slow(void) {
}

static void led_display_blink_fast(void) {
}

static void led_display_breath(void) {
}

void led_init(void) {
    led_set_mode(LED_MODE_OFF);
    xTaskNotifyWait(0, LED_MODE_CHANGE_NOTIFY, NULL, 0);
    led_task_handle = xTaskGetCurrentTaskHandle();
}

void led_main_task(void*)
{
    led_init();
    while (1) {
        switch (led_status) {
            case LED_MODE_OFF:
                xTaskNotifyWait(0, LED_MODE_CHANGE_NOTIFY, NULL, portMAX_DELAY);
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
