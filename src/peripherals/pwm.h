#ifndef __PWM_H__
#define __PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void PWM_Init(TIM_HandleTypeDef* htim, uint32_t channel);
void pwm_set_frequency(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t frequency_hz);
void pwm_set_pulsewidth_us(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t pulsewidth_us);
void pwm_set_duty_cycle_percent(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t duty_cycle_percent);
void pwm_enable(TIM_HandleTypeDef* htim, uint32_t channel);
void pwm_disable(TIM_HandleTypeDef* htim, uint32_t channel);

#ifdef __cplusplus
}
#endif
#endif /* __PWM_H__ */
