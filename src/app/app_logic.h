#ifndef __APP_LOGIC_H
#define __APP_LOGIC_H

#include "main.h"
#include <stdbool.h>
#include "dsp_utils.h" // Include for GoertzelResults_t
#include "app_config.h" // Include for ADC_BUFFER_LENGTH

// External declaration of peripheral handles
extern TIM_HandleTypeDef htim3; // PWM Timer
extern ADC_HandleTypeDef hadc1; // ADC Handle
extern UART_HandleTypeDef huart2; // UART Handle

// External declaration of ADC buffer and flags (defined in app_logic.c)
extern uint32_t g_adc_dma_buf[ADC_BUFFER_LENGTH]; // Original buffer type
extern volatile bool g_adc_conv_half_cplt;
extern volatile bool g_adc_conv_cplt;

/**
  * @brief  Runs all initial setup for the application.  
  * @retval None
  */
void app_setup(void);

/**
  * @brief  Contains the main loop logic for the application.
  * @retval None
  */
void app_loop(void);

#endif /* __APP_LOGIC_H */
