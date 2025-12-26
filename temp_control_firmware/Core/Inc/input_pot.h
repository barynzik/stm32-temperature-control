/**
 * @file input_pot.h
 * @brief User input module using potentiometer and ADC.
 *
 * This module provides an interface for reading a user-defined
 * reference value (set-point) from an analog potentiometer.
 * The potentiometer represents an additional user input device
 * used to adjust the temperature reference locally.
 */

#ifndef INC_INPUT_POT_H_
#define INC_INPUT_POT_H_

#include <stdint.h>

void Pot_Init(void);
uint16_t Pot_ReadRaw(void);
float Pot_ReadSetpointC(void);


#endif /* INC_INPUT_POT_H_ */
