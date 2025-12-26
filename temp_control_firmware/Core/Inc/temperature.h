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


/**
 * @brief Convert ADC raw value to temperature in degrees Celsius.
 * @param adc_raw ADC reading (0..4095)
 * @return Temperature in degrees Celsius
 */
void Temperature_Init(void);
float Temperature_ReadC(void);
#endif /* INC_TEMPERATURE_H_ */
