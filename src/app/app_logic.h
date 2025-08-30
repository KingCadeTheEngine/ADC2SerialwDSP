#ifndef __APP_LOGIC_H
#define __APP_LOGIC_H

#include "main.h"
#include <stdbool.h>

// External declaration of ADC buffer and flags
// These are defined in app_logic.c
extern uint32_t g_adc_dma_buf[];
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
