/**
 * @file temperature.h
 * @brief Temperature measurement module interface.
 *
 * This file declares functions and data types related to temperature
 * measurement and conversion from ADC readings to physical units.
 */


#ifndef INC_TEMPERATURE_H_
#define INC_TEMPERATURE_H_

#include <stdint.h>

float Temperature_FromRaw(uint16_t raw);
#endif /* INC_TEMPERATURE_H_ */
