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
