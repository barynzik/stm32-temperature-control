/**
 * @file input_pot.c
 * @brief Potentiometer input implementation.
 *
 * This file contains the implementation of functions used to read
 * the potentiometer value via ADC and convert it into a temperature
 * set-point within a predefined range.
 */

#include "input_pot.h"
#include "main.h"
#include "config.h"

extern ADC_HandleTypeDef hadc1;

void Pot_Init(void)
{

}

uint16_t Pot_ReadRaw(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t raw = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return raw;
}

float Pot_ReadSetpointC(void)
{
    uint16_t raw = Pot_ReadRaw();
    float x = (float)raw / 4095.0f;
    return T_SETPOINT_MIN_C + x * (T_SETPOINT_MAX_C - T_SETPOINT_MIN_C);
}
