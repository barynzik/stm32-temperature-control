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
#include <main.h>

extern UART_HandleTypeDef huart3;

/**
 * @brief Initialize UART interface (buffers/state).
 */
void UART_Init(void);

/**
 * @brief Process UART communication (receive, parse commands, update setpoint).
 *
 * This function should be called periodically from the main loop.
 * Incoming commands are validated using a checksum.
 */
void UART_Service(void);

/**
 * @brief Get current temperature setpoint [°C] set via UART (or other source).
 */
float UART_GetSetpointC(void);

/**
 * @brief Calculate XOR checksum for a message buffer.
 *
 * @param data Pointer to data buffer
 * @param len  Number of bytes included in checksum calculation
 * @return XOR checksum value
 */
uint8_t UART_CalcChecksum(const uint8_t *data, uint16_t len);

/**
 * @brief Parse SET command with checksum validation.
 *
 * Expected format example: "SET:35.0*5A"
 * where "*5A" is a hex checksum calculated over "SET:35.0".
 *
 * @param msg   Null-terminated received message (will be modified during parsing)
 * @param value Parsed setpoint value [°C]
 * @return true if checksum is valid and parsing succeeded
 */
bool UART_ParseSetpoint(char *msg, float *value);

#endif /* INC_UART_IF_H_ */
