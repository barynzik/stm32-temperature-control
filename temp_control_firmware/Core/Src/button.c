/**
 * @file button.c
 * @brief Implementation of user button handling.
 *
 * This module provides debounced button input processing and
 * detection of short and long press events.
 *
 * The button is assumed to be active-low (pressed = logic 0).
 * Timing is based on periodic calls to Button_Task_100ms().
 *
 * @author
 * Borys Ovsiyenko
 */

#include "button.h"
#include "main.h"
#include "stm32f7xx_hal.h"

#ifndef BTN_USER_Pin
  #ifdef USER_Btn_Pin
    #define BTN_USER_Pin USER_Btn_Pin
    #define BTN_USER_GPIO_Port USER_Btn_GPIO_Port
 #else
    #define BTN_USER_Pin GPIO_PIN_13
    #define BTN_USER_GPIO_Port GPIOC
  #endif
#endif

#ifndef BTN_USER_Pin
#endif

#ifndef BTN_USER_GPIO_Port
#endif

#define BTN_LONG_TICKS   10
#define BTN_DEBOUNCE_TICKS 2

static button_event_t g_evt = BTN_EVT_NONE;

void Button_Init(void)
{
    g_evt = BTN_EVT_NONE;
}

void Button_Task_100ms(void)
{
    static GPIO_PinState raw_prev      = GPIO_PIN_SET;
    static GPIO_PinState debounced     = GPIO_PIN_SET;
    static uint32_t      debounce_cnt  = 0;
    static uint32_t      hold_ticks    = 0;

    GPIO_PinState raw_now = HAL_GPIO_ReadPin(BTN_USER_GPIO_Port, BTN_USER_Pin);


    if (raw_now == raw_prev) {
        if (debounce_cnt < 0xFFFFFFFFu) {
            debounce_cnt++;
        }
    } else {
        debounce_cnt = 0;
        raw_prev = raw_now;
    }

    if (debounce_cnt >= BTN_DEBOUNCE_TICKS) {
        if (raw_now != debounced) {
            debounced = raw_now;

            if (debounced == GPIO_PIN_SET) {
                if (hold_ticks >= BTN_LONG_TICKS) {
                    if (g_evt == BTN_EVT_NONE) g_evt = BTN_EVT_LONG;
                } else {
                    if (g_evt == BTN_EVT_NONE) g_evt = BTN_EVT_SHORT;
                }
                hold_ticks = 0;
            }
            else {
                hold_ticks = 0;
            }
        }
    }

    if (debounced == GPIO_PIN_RESET) {
        if (hold_ticks < 0xFFFFFFFFu) {
            hold_ticks++;
        }
    }
}


button_event_t Button_ConsumeEvent(void)
{
    button_event_t e = g_evt;
    g_evt = BTN_EVT_NONE;
    return e;
}
