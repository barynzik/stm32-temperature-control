/**
 * @file ui_led.h
 * @brief User interface LED control module.
 *
 * This module provides functions for controlling LED indicators used as
 * an additional user output device. LEDs are used to indicate system status,
 * normal operation and alarm conditions based on the measured temperature.
 *
 * The module is hardware-independent except for GPIO definitions provided
 * by the BSP or STM32CubeMX-generated code.

 */

#pragma once
#include <stdbool.h>
#include <stdint.h>

void UI_LED_Init(void);
void UI_LED_Task_100ms(bool in_range, bool alarm);
