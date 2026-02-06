/**
 * @file heater.c
 * @brief Heater control using PWM output.
 *
 * This module provides an interface for controlling the heating element
 * by adjusting the PWM duty cycle applied to the MOSFET transistor.
 * Output saturation is applied to ensure safe operation.
 */


#include "heater.h"
#include "main.h"
extern TIM_HandleTypeDef htim1;

void Heater_Init(void)
{
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  Heater_SetDutyPercent(0.0f);
}

void Heater_SetDutyPercent(float duty)
{
  if (duty < 0.0f) duty = 0.0f;
  if (duty > 100.0f) duty = 100.0f;

  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim1);
  uint32_t ccr = (uint32_t)((duty / 100.0f) * (float)(arr + 1));
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr);
}
