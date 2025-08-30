#include "app_logic.h"
#include "app_config.h"
#include "system_config.h"
#include "gpio.h"
#include "tim.h"
#include "adc.h"
#include "uart.h"
#include "pwm.h" // Include the new PWM header
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
uint32_t g_adc_dma_buf[ADC_BUFFER_LENGTH];
volatile bool g_adc_conv_half_cplt = false;
volatile bool g_adc_conv_cplt = false;
TIM_HandleTypeDef htim3; // Declare htim3 here

/* Private function prototypes -----------------------------------------------*/
static void process_adc_data(uint32_t* data_buffer, uint16_t buffer_size);

/**
  * @brief  Application setup function. Initializes all peripherals and starts them.
  * @retval None
  */
void app_setup(void)
{
  /* MCU Configuration */
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  PWM_Init(&htim3, PWM_TIM_CHANNEL); // Initialize PWM using the new function name
  UART_Init();
  UART_Transmit_DMA("UART Initialized in app_setup\r\n");

  /* Start the timer in interrupt mode */
  if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Start ADC conversion with DMA */
  if (HAL_ADC_Start_DMA(&hadc1, g_adc_dma_buf, ADC_BUFFER_LENGTH) != HAL_OK)
  {
    Error_Handler();
  }

  /* Start PWM */
  if (HAL_TIM_PWM_Start(&htim3, PWM_TIM_CHANNEL) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  Application main loop.
  * @retval None
  */
void app_loop(void)
{
    if (g_adc_conv_half_cplt)
    {
      g_adc_conv_half_cplt = false;
      process_adc_data(&g_adc_dma_buf[0], ADC_BUFFER_LENGTH / 2);
    }

    if (g_adc_conv_cplt)
    {
      g_adc_conv_cplt = false;
      process_adc_data(&g_adc_dma_buf[ADC_BUFFER_LENGTH / 2], ADC_BUFFER_LENGTH / 2);
    }
}

/**
  * @brief  Processes a block of ADC data.
  * @param  data_buffer: Pointer to the data buffer to process.
  * @param  buffer_size: Size of the data buffer.
  * @retval None
  */
static void process_adc_data(uint32_t* data_buffer, uint16_t buffer_size)
{
  uint32_t sum = 0;
  for (uint16_t i = 0; i < buffer_size; i++)
  {
    sum += data_buffer[i];
  }
  uint16_t avg_adc_value = sum / buffer_size;

  static char tx_buffer[50]; // A buffer to store the formatted string
  sprintf(tx_buffer, "Mean: %hu\r\n", avg_adc_value);

  UART_Transmit_DMA(tx_buffer);
}
