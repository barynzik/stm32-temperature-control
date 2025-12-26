/**
 * @file uart_if.c
 * @brief Serial communication interface.
 *
 * This module handles UART-based communication with a PC or other external
 * device. It allows setting the temperature reference value and reading
 * current measurement, reference and control signals.
 */

#include "uart_if.h"

static float g_setpoint_c = 35.0f;

void UART_Init(void)
{
    // TODO
}

void UART_Service(void)
{
    // TODO
}
float UART_GetSetpointC(void)
{
    return g_setpoint_c;
}
