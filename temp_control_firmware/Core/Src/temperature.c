/**
 * @file temperature.c
 * @brief Temperature measurement using NTC and ADC.
 *
 * This module implements temperature measurement based on an NTC thermistor.
 * It converts raw ADC values into temperature expressed in degrees Celsius
 * using a mathematical model of the thermistor.
 */

#include "temperature.h"

/**
 * @brief Convert ADC raw value to temperature in degrees Celsius.
 */
float TEMP_AdcToCelsius(uint16_t adc_raw)
{
    (void)adc_raw;
    return 0.0f;  // TODO: implement conversion
}
