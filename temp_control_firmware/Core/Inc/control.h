/**
 * @file control.h
 * @brief Control algorithm module interface.
 *
 * This file provides the interface for the PI/PID controller used in
 * the temperature control system.
 */

#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

/**
 * @brief Compute PI controller output.
 */
float Control_Update(float ref_c, float meas_c);
void Control_Init(void);



#endif /* INC_CONTROL_H_ */
