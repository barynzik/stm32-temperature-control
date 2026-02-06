/**
 * @file ui_led.c
 * @brief Implementation of LED-based user interface.
 *
 * This file contains the implementation of LED control logic used for
 * system status indication. The LED provides a heartbeat signal during
 * normal operation and visual alarm indication when the process value
 * exceeds the safe operating range.
 *
 * The LED update function is intended to be called periodically from the
 * main control loop or a timer interrupt.
 */

#include "ui_led.h"
#include "main.h"

static uint32_t tick = 0;

void UI_LED_Init(void)
{
    tick = 0;
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET); // стартуем с LED OFF
}

void UI_LED_Task_100ms(bool in_range, bool alarm)
{
    tick++;

    if (alarm) {
        // Alarm: fast blink ~5 Hz (toggle every 100 ms -> 5 Hz)
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        return;
    }

    if (in_range) {
        // Normal safe range: solid ON
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        return;
    }

    // Out of safe range (but not alarm): heartbeat 1 Hz
    if ((tick % 10) == 0) {
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
}

