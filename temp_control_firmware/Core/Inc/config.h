/**
 * @file config.h
 * @brief System configuration parameters.
 *
 * This file contains configuration constants such as control range,
 * controller parameters and hardware-related settings. It separates
 * system configuration from application logic.
 */

#pragma once

// Control range
#define T_MIN_C        20.0f
#define T_MAX_C        60.0f

// Sampling
#define CONTROL_TS_S   0.1f   // 100 ms

// PI gains
#define KP             2.0f
#define KI             0.5f

// ADC / NTC parameters
#define ADC_VREF       3.3f
#define ADC_MAX        4095.0f
#define R_FIXED        10000.0f
#define NTC_BETA       3950.0f
#define NTC_R0         10000.0f
#define NTC_T0_K       298.15f
