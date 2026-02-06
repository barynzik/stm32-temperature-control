/**
 * @file uart_if.h
 * @brief UART communication module interface.
 *
 * This file declares functions related to serial communication,
 * command parsing and telemetry transmission.
 */

#ifndef INC_UART_IF_H_
#define INC_UART_IF_H_

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

void UARTIF_Init(void);
void UARTIF_Task(void);
bool UARTIF_HasSetpoint(void);
float UARTIF_GetSetpointC(void);
bool UARTIF_ConsumeTelemetryRequest(void);
void UARTIF_SendTelemetry(float t_meas, float t_ref, float pwm);


#endif /* INC_UART_IF_H_ */
