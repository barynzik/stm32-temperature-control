/**
 * @file button.h
 * @brief User button handling module.
 *
 * This module implements debounced user button handling with
 * detection of short and long press events.
 *
 * The button state is processed periodically by calling
 * Button_Task_100ms(), which generates button events that can be
 * consumed by the application logic.
 *
 * Typical usage:
 *  - Short press  : increase temperature setpoint
 *  - Long press   : decrease temperature setpoint
 *
 * @author
 * Borys Ovsiyenko
 */
#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include "main.h"


typedef enum {
    BTN_EVT_NONE = 0,
    BTN_EVT_SHORT,
    BTN_EVT_LONG
} button_event_t;

void Button_Init(void);

void Button_Task_100ms(void);

button_event_t Button_ConsumeEvent(void);

#endif // BUTTON_H
