/**
 * @file temperature.c
 * @brief Temperature measurement using NTC and ADC.
 *
 * This module implements temperature measurement based on an NTC thermistor.
 * It converts raw ADC values into temperature expressed in degrees Celsius
 * using a mathematical model of the thermistor.
 */

#include "temperature.h"
#include <math.h>
#include "config.h"

float Temperature_FromRaw(uint16_t raw)
{
  if (raw <= 0) raw = 1;
  if (raw >= 4095) raw = 4094;

  float v = ((float)raw / 4095.0f) * ADC_VREF;

  float r_ntc = R_FIXED * (v / (ADC_VREF - v));

  float invT = (1.0f / NTC_T0_K) + (1.0f / NTC_BETA) * logf(r_ntc / NTC_R0);
  float T = 1.0f / invT;

  return T - 273.15f;
}
