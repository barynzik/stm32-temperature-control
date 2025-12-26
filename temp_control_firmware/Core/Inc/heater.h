/**
 * @file heater.h
 * @brief Heater control module interface.
 *
 * This file declares functions for controlling the heating element
 * using PWM signals.
 */

#ifndef INC_HEATER_H_
#define INC_HEATER_H_

/**
 * @brief Set heater PWM duty cycle.
 */
void Heater_Init(void);
void Heater_SetDutyPercent(float duty);
#endif /* INC_HEATER_H_ */
