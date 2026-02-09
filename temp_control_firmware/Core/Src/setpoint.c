/**
 * @file setpoint.c
 * @brief Implementation of temperature setpoint management.
 *
 * This file contains the implementation of the temperature setpoint logic.
 * The setpoint value is stored internally and can be modified only through
 * the provided API functions.
 *
 * All setpoint values are constrained to a predefined safe range in order
 * to prevent unsafe operating conditions.
 *
 * Typical sources of setpoint changes:
 *  - UART commands
 *  - User button input
 *  - PC GUI
 *
 * @author
 * Borys Ovsiyenko
 */
#include "setpoint.h"
#include "config.h"

static volatile float g_setpoint_c = T_SETPOINT_DEFAULT_C;

float Setpoint_GetC(void)
{
    return g_setpoint_c;
}

void Setpoint_SetC(float value_c)
{
    if (value_c < T_SAFE_MIN_C) value_c = T_SAFE_MIN_C;
    if (value_c > T_SAFE_MAX_C) value_c = T_SAFE_MAX_C;

    g_setpoint_c = value_c;

}
