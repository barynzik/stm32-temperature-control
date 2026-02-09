/**
 * @file uart_if.c
 * @brief UART communication interface implementation.
 *
 * This module implements a simple line-based UART protocol for
 * communication with an external PC application (terminal, Python GUI,
 * MATLAB logger, etc.).
 *
 * Supported commands:
 *  - "T<value>"  : set temperature setpoint (°C)
 *  - "?"         : request telemetry data
 *
 * Telemetry format (JSON, no CRC):
 *  {"T_meas":xx.xx,"T_ref":yy.yy,"PWM":zz.z}
 *
 * UART reception is interrupt-driven and uses a line buffer terminated
 * by CR or LF characters.
 *
 * @author
 * Borys Ovsiyenko
 */

#include "uart_if.h"
#include "setpoint.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <stddef.h>
#include <stdint.h>

#ifndef UARTIF_HUART
#define UARTIF_HUART huart3
#endif

extern UART_HandleTypeDef UARTIF_HUART;

static uint8_t  rx_byte;
static char     line_buf[64];
static uint32_t line_len = 0;

static volatile bool  telemetry_req     = false;
static volatile bool  has_setpoint      = false;
static volatile float last_setpoint_c   = 0.0f;
static volatile bool  line_ready        = false;

/* ===================== UART RX ===================== */

static void start_rx_it(void)
{
    HAL_UART_Receive_IT(&UARTIF_HUART, &rx_byte, 1);
}

void UARTIF_Init(void)
{
    line_len = 0;
    line_buf[0] = '\0';

    telemetry_req   = false;
    has_setpoint    = false;
    last_setpoint_c = 0.0f;
    line_ready      = false;

    start_rx_it();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != &UARTIF_HUART) return;

    if (line_ready) {
        start_rx_it();
        return;
    }

    char c = (char)rx_byte;

    if (c == '\r' || c == '\n') {
        if (line_len > 0) {
            line_buf[line_len] = '\0';
            line_ready = true;
        }
    } else {
        if (line_len < (sizeof(line_buf) - 1U)) {
            line_buf[line_len++] = c;
            line_buf[line_len] = '\0';
        } else {
            line_len = 0;
            line_buf[0] = '\0';
        }
    }


    start_rx_it();
}

/* ===================== Command handling ===================== */

static void handle_line(const char *s)
{
    while (*s && isspace((unsigned char)*s)) s++;

    if (s[0] == 'T') {
        float v = (float)atof(&s[1]);

        Setpoint_SetC(v);
        has_setpoint = true;
        last_setpoint_c = v;

        const char *ok = "OK\n";
        HAL_UART_Transmit(&UARTIF_HUART, (uint8_t*)ok, (uint16_t)strlen(ok), 50);
        return;
    }

    if (s[0] == '?') {
        telemetry_req = true;
        const char *ok = "OK\n";
        HAL_UART_Transmit(&UARTIF_HUART, (uint8_t*)ok, (uint16_t)strlen(ok), 50);
        return;
    }

    const char *err = "ERR\n";
    HAL_UART_Transmit(&UARTIF_HUART, (uint8_t*)err, (uint16_t)strlen(err), 50);
}

void UARTIF_Task(void)
{
    if (!line_ready) return;

    char tmp[64];
    strncpy(tmp, line_buf, sizeof(tmp) - 1U);
    tmp[sizeof(tmp) - 1U] = '\0';

    line_len = 0;
    line_buf[0] = '\0';
    line_ready = false;

    handle_line(tmp);
}


/* ===================== Public getters ===================== */

bool UARTIF_HasSetpoint(void)
{
    return has_setpoint;
}

float UARTIF_GetSetpointC(void)
{
    return last_setpoint_c;
}

bool UARTIF_ConsumeTelemetryRequest(void)
{
    if (telemetry_req) {
        telemetry_req = false;
        return true;
    }
    return false;
}

/* ===================== Telemetry TX ===================== */

void UARTIF_SendTelemetry(float t_meas, float t_ref, float pwm)
{
    char frame[128];
    int n = snprintf(frame, sizeof(frame),
                     "{\"T_meas\":%.2f,\"T_ref\":%.2f,\"PWM\":%.1f}\r\n",
                     (double)t_meas, (double)t_ref, (double)pwm);
    if (n < 0 || n >= (int)sizeof(frame)) return;

    HAL_UART_Transmit(&UARTIF_HUART, (uint8_t*)frame, (uint16_t)strlen(frame), 100);
}



