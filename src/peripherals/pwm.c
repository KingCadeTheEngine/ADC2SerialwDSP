#include "pwm.h"
#include "app_config.h"
#include "tim.h" // For HAL_TIM_PWM_MspInit and Error_Handler
#include "main.h" // For Error_Handler

// TIM_HandleTypeDef htim3; // Define htim3 here (now passed as parameter)

// Store last frequency and duty cycle for each channel (assuming max 4 channels)
static uint32_t last_frequency[4] = {PWM_DEFAULT_FREQUENCY_HZ, PWM_DEFAULT_FREQUENCY_HZ, PWM_DEFAULT_FREQUENCY_HZ, PWM_DEFAULT_FREQUENCY_HZ};
static uint32_t last_duty_cycle[4] = {PWM_DEFAULT_DUTY_CYCLE_PERCENT, PWM_DEFAULT_DUTY_CYCLE_PERCENT, PWM_DEFAULT_DUTY_CYCLE_PERCENT, PWM_DEFAULT_DUTY_CYCLE_PERCENT};

void PWM_Init(TIM_HandleTypeDef* htim, uint32_t channel)
{
  htim->Instance = PWM_TIM_INSTANCE;
  htim->Init.CounterMode = TIM_COUNTERMODE_UP;
  htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(htim) != HAL_OK)
  {
    Error_Handler();
  }

  last_frequency[channel] = PWM_DEFAULT_FREQUENCY_HZ;
  last_duty_cycle[channel] = PWM_DEFAULT_DUTY_CYCLE_PERCENT;
  pwm_set_frequency(htim, channel, PWM_DEFAULT_FREQUENCY_HZ);
  pwm_set_duty_cycle_percent(htim, channel, PWM_DEFAULT_DUTY_CYCLE_PERCENT);
}

void pwm_set_frequency(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t frequency_hz)
{
  if (frequency_hz == 0) return; // Avoid division by zero

  uint32_t tim_clock = HAL_RCC_GetPCLK1Freq() * 2; // TIM3 is on APB1, timer clock is twice PCLK1 if APB1 prescaler is not 1
  uint32_t period = (tim_clock / frequency_hz) - 1;

  // Find a suitable prescaler to keep period within 16-bit range
  uint32_t prescaler = 0;
  while (period > 65535 && prescaler < 65535)
  {
    prescaler++;
    period = (tim_clock / (frequency_hz * (prescaler + 1))) - 1;
  }

  if (prescaler >= 65535 || period >= 65535) {
    // Frequency too high or too low to be achieved
    return;
  }

  htim->Init.Prescaler = prescaler;
  htim->Init.Period = period;
  HAL_TIM_Base_Init(htim);

  last_frequency[channel] = frequency_hz;
  // Use last duty cycle for this channel
  uint32_t duty_cycle_percent = last_duty_cycle[channel];
  uint32_t pulse = (htim->Init.Period + 1) * duty_cycle_percent / 100;

  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, channel);
}

void pwm_set_pulsewidth_us(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t pulsewidth_us)
{
  uint32_t tim_clock = HAL_RCC_GetPCLK1Freq() * 2; // TIM3 is on APB1, timer clock is twice PCLK1 if APB1 prescaler is not 1
  uint32_t pulse = (tim_clock / (htim->Init.Prescaler + 1)) * pulsewidth_us / 1000000;

  if (pulse > htim->Init.Period) {
    pulse = htim->Init.Period; // Cap pulse to period
  }

  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, channel);
  // HAL_TIM_PWM_Start(htim, channel); // Removed as per user request
}

void pwm_set_duty_cycle_percent(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t duty_cycle_percent)
{
  if (duty_cycle_percent > 100) {
    duty_cycle_percent = 100; // Cap duty cycle at 100%
  }

  last_duty_cycle[channel] = duty_cycle_percent;
  // Use last frequency for this channel
  uint32_t frequency_hz = last_frequency[channel];
  uint32_t tim_clock = HAL_RCC_GetPCLK1Freq() * 2;
  uint32_t period = (tim_clock / frequency_hz) - 1;
  uint32_t prescaler = 0;
  while (period > 65535 && prescaler < 65535)
  {
    prescaler++;
    period = (tim_clock / (frequency_hz * (prescaler + 1))) - 1;
  }
  if (prescaler >= 65535 || period >= 65535) {
    return;
  }
  htim->Init.Prescaler = prescaler;
  htim->Init.Period = period;
  HAL_TIM_Base_Init(htim);

  uint32_t pulse = (period + 1) * duty_cycle_percent / 100;

  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, channel);
}

void pwm_enable(TIM_HandleTypeDef* htim, uint32_t channel)
{
  HAL_TIM_PWM_Start(htim, channel);
}

void pwm_disable(TIM_HandleTypeDef* htim, uint32_t channel)
{
  HAL_TIM_PWM_Stop(htim, channel);
}
