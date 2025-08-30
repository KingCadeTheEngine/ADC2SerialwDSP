#include "app_logic.h"
#include "app_config.h"
#include "system_config.h"
#include "gpio.h"
#include "tim.h"
#include "adc.h"
#include "uart.h"
#include "pwm.h"
#include "dsp_utils.h" // Include DSP utilities
#include <stdio.h>
#include <string.h>
#include <math.h> // For floating point operations if needed

/* Private variables ---------------------------------------------------------*/
uint32_t g_adc_dma_buf[ADC_BUFFER_LENGTH]; // ADC DMA buffer (uint32_t to match DMA transfer width)
volatile bool g_adc_conv_half_cplt = false; // Flag for ADC half conversion complete
volatile bool g_adc_conv_cplt = false;      // Flag for ADC full conversion complete

/* Private function prototypes -----------------------------------------------*/
// No longer need process_adc_data, it's integrated into app_loop

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
  PWM_Init(&htim3, PWM_TIM_CHANNEL);
  UART_Init();
  DSP_Init(); // Initialize DSP module

  /* Start the timer in interrupt mode */
  if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Start ADC conversion with DMA */
  // Note: g_adc_dma_buf is uint32_t, but ADC is 12-bit (uint16_t). 
  // Assuming DMA packs two 12-bit samples into one 32-bit word, or 
  // the ADC is configured for 32-bit data width. 
  // DSP_ProcessWindow expects uint16_t*, so we'll cast when passing.
  if (HAL_ADC_Start_DMA(&hadc1, g_adc_dma_buf, ADC_BUFFER_LENGTH) != HAL_OK)
  {
    Error_Handler();
  }

  /* Start PWM */
  if (HAL_TIM_PWM_Start(&htim3, PWM_TIM_CHANNEL) != HAL_OK)
  {
    Error_Handler();
  }

  /* Start UART Reception */
  UART_Receive_DMA_Start();

  /* Transmit initial message */
  const char* msg = "App Setup Complete. UART RX running.\r\n";
  UART_Transmit_DMA((uint8_t*)msg, strlen(msg));
}

/**
  * @brief  Application main loop.
  * @retval None
  */
void app_loop(void)
{

  // Process ADC data when a half or full buffer is complete
  if (g_adc_conv_half_cplt || g_adc_conv_cplt)
  {
    GoertzelResults_t results;
    char tx_buffer[128]; // Buffer for formatted output
    uint16_t *adc_window_ptr = NULL;
    uint32_t window_start_idx = 0;

    // Determine which half of the buffer to process
    if (g_adc_conv_half_cplt)
    {
        window_start_idx = 0; // First half of the buffer
        g_adc_conv_half_cplt = false; // Clear the flag
    }
    else if (g_adc_conv_cplt)
    {
        window_start_idx = ADC_BUFFER_LENGTH / 2; // Second half of the buffer
        g_adc_conv_cplt = false; // Clear the flag
    }

    // Cast uint32_t buffer to uint16_t for DSP processing
    // This assumes each uint32_t contains two uint16_t samples, or that
    // the lower 16 bits of each uint32_t contain the sample. 
    // Adjust if your ADC DMA configuration is different.
    adc_window_ptr = (uint16_t*)&g_adc_dma_buf[window_start_idx];

    // Process the 4 windows (64 samples each) within the 256-sample half-buffer
    for (int i = 0; i < (ADC_BUFFER_LENGTH / 2) / DSP_WINDOW_LENGTH; i++) {
        if (DSP_ProcessWindow(&adc_window_ptr[i * DSP_WINDOW_LENGTH], DSP_WINDOW_LENGTH, &results))
        {
            // Format the results into the desired string format: xx,xx,xx,xx
            // Example: Mag39,Phase39,Mag40,Phase40
            // Scale phase from radians to degrees (0-360) and then to integer for transmission
            int mag39 = (int)(results.magnitude_39khz * 1000.0f); // Scale for precision
            int phase39 = (int)(results.phase_39khz * 180.0f / 3.14159265359f); // Convert to degrees
            int mag40 = (int)(results.magnitude_40khz * 1000.0f);
            int phase40 = (int)(results.phase_40khz * 180.0f / 3.14159265359f);
            int mag41 = (int)(results.magnitude_41khz * 1000.0f);
            int phase41 = (int)(results.phase_41khz * 180.0f / 3.14159265359f);

            // Ensure phase is positive (0-360)
            if (phase39 < 0) phase39 += 360;
            if (phase40 < 0) phase40 += 360;
            if (phase41 < 0) phase41 += 360;

            int len = snprintf(tx_buffer, sizeof(tx_buffer), "%d,%d,%d,%d,%d,%d\r\n", 
                                mag39, phase39, mag40, phase40, mag41, phase41);
            
            if (len > 0) {
                UART_Transmit_DMA((uint8_t*)tx_buffer, len);
            }
        }
    }
  }
}
