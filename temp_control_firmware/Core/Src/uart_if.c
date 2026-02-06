#include "uart_if.h"
#include "setpoint.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <stddef.h>
#include <stdint.h>
// #include <stdbool.h>  // включи, если bool не определён из uart_if.h

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

/* ===================== CRC8 helpers ===================== */

// CRC-8 (poly 0x07), init 0x00, MSB-first
static uint8_t crc8_0x07(const uint8_t *data, size_t len)
{
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

static int hex_nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

static int parse_hex2(const char *p, uint8_t *out) // p -> "AB"
{
    int hi = hex_nibble(p[0]);
    int lo = hex_nibble(p[1]);
    if (hi < 0 || lo < 0) return 0;
    *out = (uint8_t)((hi << 4) | lo);
    return 1;
}

// Ожидает формат: "<payload>*<2hex>"
// Если CRC OK — режет строку до payload (ставит '\0' на '*') и возвращает 1, иначе 0.
static int verify_and_strip_crc(char *line)
{
    char *star = strchr(line, '*');
    if (!star) return 0;                       // нет CRC
    if (star[1] == '\0' || star[2] == '\0') return 0; // нет 2 hex
    if (star[3] != '\0') return 0;             // после CRC ничего быть не должно

    uint8_t rx_crc = 0;
    if (!parse_hex2(star + 1, &rx_crc)) return 0;

    *star = '\0'; // теперь line == payload

    uint8_t calc = crc8_0x07((const uint8_t*)line, strlen(line));
    return (calc == rx_crc);
}

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

// Вызывается HAL-ом, когда пришёл 1 байт
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != &UARTIF_HUART) return;

    // Пока строка не обработана в main — не копим новые данные (иначе будет гонка).
    // Здесь просто дропаем байты до обработки строки.
    if (line_ready) {
        start_rx_it();
        return;
    }

    char c = (char)rx_byte;

    if (c == '\r' || c == '\n') {
        // завершили строку (не делаем пустые строки)
        if (line_len > 0) {
            line_buf[line_len] = '\0';
            line_ready = true;
        }
    } else {
        // добавляем символ, если есть место (оставляем 1 байт под '\0')
        if (line_len < (sizeof(line_buf) - 1U)) {
            line_buf[line_len++] = c;
            line_buf[line_len] = '\0';
        } else {
            // переполнение — сброс буфера
            line_len = 0;
            line_buf[0] = '\0';
        }
    }


    start_rx_it();
}

/* ===================== Command handling ===================== */

static void handle_line(const char *s)
{
    // пропустим ведущие пробелы
    while (*s && isspace((unsigned char)*s)) s++;

    // формат: T35.0  (setpoint)
    if (s[0] == 'T') {
        float v = (float)atof(&s[1]);

        Setpoint_SetC(v);
        has_setpoint = true;
        last_setpoint_c = v;

        const char *ok = "OK\n";
        HAL_UART_Transmit(&UARTIF_HUART, (uint8_t*)ok, (uint16_t)strlen(ok), 50);
        return;
    }

    // формат: ? (запрос телеметрии)
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

    // копируем строку в локальный буфер, чтобы спокойно обрабатывать
    char tmp[64];
    strncpy(tmp, line_buf, sizeof(tmp) - 1U);
    tmp[sizeof(tmp) - 1U] = '\0';

    // освобождаем приемный буфер как можно раньше
    line_len = 0;
    line_buf[0] = '\0';
    line_ready = false;

    // обработка команды с CRC
    if (!verify_and_strip_crc(tmp)) {
        const char *err = "ERR:CRC\n";
        HAL_UART_Transmit(&UARTIF_HUART, (uint8_t*)err, (uint16_t)strlen(err), 50);
        return;
    }

    // tmp уже содержит только payload: "T35.0" или "?"
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
    char payload[96];
    int n = snprintf(payload, sizeof(payload),
                     "{\"T_meas\":%.2f,\"T_ref\":%.2f,\"PWM\":%.1f}",
                     (double)t_meas, (double)t_ref, (double)pwm);
    if (n < 0 || n >= (int)sizeof(payload)) {
        return;
    }

    uint8_t crc = crc8_0x07((const uint8_t*)payload, (uint32_t)strlen(payload));

    char frame[128];
    int m = snprintf(frame, sizeof(frame), "%s*%02X\r\n", payload, (unsigned)crc);
    if (m < 0 || m >= (int)sizeof(frame)) {
        return;
    }

    HAL_UART_Transmit(&huart3, (uint8_t*)frame, (uint16_t)strlen(frame), 100);
}

