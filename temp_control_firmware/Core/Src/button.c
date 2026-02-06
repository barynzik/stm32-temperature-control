#include "button.h"
#include "main.h"   // тут обычно объявлены BTN_USER_Pin и BTN_USER_GPIO_Port
#include "stm32f7xx_hal.h"  // на всякий случай (GPIO_PinState, HAL_GPIO_ReadPin)

#ifndef BTN_USER_Pin
#warning "BTN_USER_Pin not defined. Set User Label for your button pin in CubeMX (e.g. BTN_USER)."
#endif

#ifndef BTN_USER_GPIO_Port
#warning "BTN_USER_GPIO_Port not defined. Set User Label for your button pin in CubeMX (e.g. BTN_USER)."
#endif

// Настройки
#define BTN_LONG_TICKS   10   // 10 * 100ms = 1s
#define BTN_DEBOUNCE_TICKS 2  // 2 * 100ms = 200ms (простая защита от дребезга)

static button_event_t g_evt = BTN_EVT_NONE;

void Button_Init(void)
{
    g_evt = BTN_EVT_NONE;
}

void Button_Task_100ms(void)
{
    // Предполагаем Pull-Up и кнопка на GND:
    // не нажата = GPIO_PIN_SET, нажата = GPIO_PIN_RESET

    static GPIO_PinState raw_prev      = GPIO_PIN_SET;  // предыдущее "сырое" чтение
    static GPIO_PinState debounced     = GPIO_PIN_SET;  // отфильтрованное состояние
    static uint32_t      debounce_cnt  = 0;
    static uint32_t      hold_ticks    = 0;

    GPIO_PinState raw_now = HAL_GPIO_ReadPin(BTN_USER_GPIO_Port, BTN_USER_Pin);

    // 1) Считаем, сколько тиков подряд raw_now держится одинаковым
    if (raw_now == raw_prev) {
        if (debounce_cnt < 0xFFFFFFFFu) {
            debounce_cnt++;
        }
    } else {
        debounce_cnt = 0;
        raw_prev = raw_now;
    }

    // 2) Если состояние стабильно N тиков — обновляем debounced
    if (debounce_cnt >= BTN_DEBOUNCE_TICKS) {
        if (raw_now != debounced) {
            debounced = raw_now;

            // Переход: отпустили (RESET -> SET)
            if (debounced == GPIO_PIN_SET) {
                if (hold_ticks >= BTN_LONG_TICKS) {
                    if (g_evt == BTN_EVT_NONE) g_evt = BTN_EVT_LONG;
                } else {
                    if (g_evt == BTN_EVT_NONE) g_evt = BTN_EVT_SHORT;
                }
                hold_ticks = 0;
            }
            // Переход: нажали (SET -> RESET)
            else {
                hold_ticks = 0;
            }
        }
    }

    // 3) Если кнопка реально удерживается (debounced = RESET) — считаем длительность
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
