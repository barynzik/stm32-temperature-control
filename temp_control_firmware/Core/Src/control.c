/**
 * @file control.c
 * @brief PI/PID control algorithm for temperature control.
 *
 * This module implements a discrete-time PI (or PID) controller used to
 * regulate the temperature of the heating element. The controller output
 * is limited to a safe range suitable for PWM control.
 */

#include "control.h"

#include "control.h"

static float kp = 3.0f;
static float ki = 0.15f;
static float integ = 0.0f;

void Control_Init(void)
{
    integ = 0.0f;
}

float Control_Update(float ref_c, float meas_c)
{
    float e = ref_c - meas_c;
    integ += e;
    float u = kp * e + ki * integ;

    if (u < 0.0f) u = 0.0f;
    if (u > 100.0f) u = 100.0f;
    return u;
}
