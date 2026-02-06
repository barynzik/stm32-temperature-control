/**
 * @file fan.c
 * @brief Cooling fan control implementation.
 *
 * This file contains the implementation of the cooling fan control
 * functions. The fan represents an additional control output device
 * used to assist the main heating element in temperature regulation.
 *
 * The fan control logic is executed in the main application loop,
 * while this module only handles low-level output control.
 *
 */
#include "fan.h"
#include "main.h"
void Fan_Set(bool on)
{
    HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin,
                      on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

