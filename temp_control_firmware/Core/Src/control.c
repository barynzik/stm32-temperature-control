/**
 * @file control.c
 * @brief PI/PID control algorithm for temperature control.
 *
 * This module implements a discrete-time PI (or PID) controller used to
 * regulate the temperature of the heating element. The controller output
 * is limited to a safe range suitable for PWM control.
 */

#include "control.h"

float CTRL_Update(float ref, float meas)
{
    (void)ref;
    (void)meas;
    return 0.0f; // TODO
}
