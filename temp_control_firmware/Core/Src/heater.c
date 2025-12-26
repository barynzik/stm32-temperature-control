/**
 * @file heater.c
 * @brief Heater control using PWM output.
 *
 * This module provides an interface for controlling the heating element
 * by adjusting the PWM duty cycle applied to the MOSFET transistor.
 * Output saturation is applied to ensure safe operation.
 */


#include "heater.h"

void HEATER_SetDuty(float duty)
{
    (void)duty;
    // TODO
}
