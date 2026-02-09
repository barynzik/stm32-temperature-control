/**
 * @file setpoint.h
 * @brief Temperature setpoint management module.
 *
 * This module provides a safe and centralized interface for managing
 * the temperature setpoint used by the control algorithm.
 *
 * The setpoint is automatically limited to a predefined safe range,
 * regardless of the source of change (UART, button, GUI, etc.).
 *
 * @author
 * Borys Ovsiyenko
 */

#ifndef INC_SETPOINT_H_
#define INC_SETPOINT_H_

float Setpoint_GetC(void);
void  Setpoint_SetC(float value_c);

#endif /* INC_SETPOINT_H_ */
