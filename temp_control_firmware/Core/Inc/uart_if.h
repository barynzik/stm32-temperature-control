/**
 * @file uart_if.h
 * @brief UART communication module interface.
 *
 * This file declares functions related to serial communication,
 * command parsing and telemetry transmission.
 */

#ifndef INC_UART_IF_H_
#define INC_UART_IF_H_

/**
 * @brief Process UART communication.
 */
void UART_Service(void);

#endif /* INC_UART_IF_H_ */
