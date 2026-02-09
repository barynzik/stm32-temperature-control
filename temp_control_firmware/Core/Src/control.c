/**
 * @file control.c
 * @brief PI/PID control algorithm for temperature control.
 *
 * This module implements a discrete-time PI (or PID) controller used to
 * regulate the temperature of the heating element. The controller output
 * is limited to a safe range suitable for PWM control.
 */

#include "control.h"
#include "config.h"

static float integ = 0.0f;

void Control_Init(void)
{
    integ = 0.0f;
}

float Control_Update(float ref_c, float meas_c)
{
    float e = ref_c - meas_c;

    float u_unsat = KP * e + KI * integ;

    float u = u_unsat;
    if (u < 0.0f)   u = 0.0f;
    if (u > 100.0f) u = 100.0f;

    bool sat_high = (u >= 100.0f) && (e > 0.0f);
    bool sat_low  = (u <= 0.0f)   && (e < 0.0f);

    if (!(sat_high || sat_low))
    {
        integ += e * CONTROL_TS_S;
    }

    return u;
}
