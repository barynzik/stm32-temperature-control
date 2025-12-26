/**
 * @file uart_if.c
 * @brief Serial communication interface.
 *
 * This module handles UART-based communication with a PC or other external
 * device. It allows setting the temperature reference value and reading
 * current measurement, reference and control signals.
 */

#include "uart_if.h"
#include <string.h>
#include <stdlib.h>


static float g_setpoint_c = 35.0f;

void UART_Init(void)
{
    // TODO
}

void UART_Service(void)
{
    static char rx_line[64];
    static uint16_t idx = 0;

    uint8_t ch;

    while (HAL_UART_Receive(&huart3, &ch, 1, 0) == HAL_OK)
    {
        if (ch == '\r')
        {
            continue;
        }

        if (ch == '\n')
        {
            rx_line[idx] = '\0';
            idx = 0;

            float sp;
            if (UART_ParseSetpoint(rx_line, &sp))
            {
                g_setpoint_c = sp;
            }
            else
            {

            }

            rx_line[0] = '\0';
        }
        else
        {
            if (idx < (sizeof(rx_line) - 1))
            {
                rx_line[idx++] = (char)ch;
            }
            else
            {
                idx = 0;
            }
        }
    }
}

float UART_GetSetpointC(void)
{
    return g_setpoint_c;
}

/**
 * @brief Calculate XOR checksum for a data buffer.
 */
uint8_t UART_CalcChecksum(const uint8_t *data, uint16_t len)
{
    uint8_t cs = 0;
    for (uint16_t i = 0; i < len; i++)
        cs ^= data[i];
    return cs;
}

bool UART_ParseSetpoint(char *msg, float *value)
{
    char *star = strchr(msg, '*');
    if (!star) return false;

    uint8_t rx_cs = (uint8_t)strtoul(star + 1, NULL, 16);

    uint8_t calc_cs = UART_CalcChecksum((const uint8_t*)msg, (uint16_t)(star - msg));
    if (rx_cs != calc_cs) return false;

    *star = '\0';

    if (strncmp(msg, "SET:", 4) != 0) return false;
    *value = (float)atof(msg + 4);
    return true;
}

