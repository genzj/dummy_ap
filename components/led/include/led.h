#pragma once


#ifdef __cplusplus
extern "C" {
#endif

enum LED_MODE {
    LED_MODE_OFF,
    LED_MODE_ON,
    LED_MODE_BLINK_SLOW,
    LED_MODE_BLINK_FAST,
    LED_MODE_BREATH,
};

/** Initialize LED IO, controller and other internal structures */
void led_init(void);
/** The LED control task, which should be wrapped into a OS task from the main component */
void led_main_task(void*);
/** Gets the current LED mode */
enum LED_MODE led_get_mode(void);
/** Sets the LED mode */
void led_set_mode(enum LED_MODE mode);

#ifdef __cplusplus
}
#endif
