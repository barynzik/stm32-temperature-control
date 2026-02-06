#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

typedef enum {
    BTN_EVT_NONE = 0,
    BTN_EVT_SHORT,
    BTN_EVT_LONG
} button_event_t;

void Button_Init(void);

/**
 * @brief Вызывать строго раз в 100 ms.
 */
void Button_Task_100ms(void);

button_event_t Button_ConsumeEvent(void);

#endif // BUTTON_H
