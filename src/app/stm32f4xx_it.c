#include "main.h"
#include "stm32f4xx_it.h"
#include "tim.h"
#include "adc.h"
#include "uart.h"
#include "pwm.h" // Added
#include <stdlib.h> // Added
#include <string.h> // Added

/* USER CODE BEGIN Includes */
#include "app_logic.h"
/* USER CODE END Includes */

extern UART_HandleTypeDef huart2;
extern volatile bool g_uart_tx_busy;
extern TIM_HandleTypeDef htim3; // Added

volatile bool g_uart_error_flag = false; // Define and initialize the error flag

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/**
  * @brief  UART error callback.
  * @param  huart: UART handle.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART2)
  {
    g_uart_error_flag = true; // Set the flag on UART error
    // Optionally, clear the error flags in the UART peripheral if needed
    // __HAL_UART_CLEAR_PEFLAG(huart);
    // __HAL_UART_CLEAR_FEFLAG(huart);
    // __HAL_UART_CLEAR_NEFLAG(huart);
    // __HAL_UART_CLEAR_OREFLAG(huart);
  }
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim2);
}

/**
  * @brief This function handles DMA2 stream0 global interrupt.
  */
void ADC_DMA_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hadc1.DMA_Handle);
}

/**
  * @brief This function handles DMA1 stream6 global interrupt (USART2_TX). // Modified comment
  */
void USARTx_TX_DMA_IRQHandler(void) // Renamed
{
  HAL_DMA_IRQHandler(huart2.hdmatx);
}

/**
  * @brief This function handles DMA1 stream5 global interrupt (USART2_RX). // Added
  */
void USARTx_RX_DMA_IRQHandler(void) // Added
{
  HAL_DMA_IRQHandler(huart2.hdmarx);
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);
}

/******************************************************************************/
/* STM32F4xx HAL Interrupt Callbacks                                          */
/******************************************************************************/

/**
  * @brief  Tx Transfer completed callback.
  * @param  huart: UART handle.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART2)
  {
    g_uart_tx_busy = false;
  }
}

/**
  * @brief  UART RX Event callback. // Added
  * @param  huart: UART handle
  * @param  Size: Number of data received
  * @retval None
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) // Added
{
    if (huart->Instance == USARTx)
    {
        // A simple parser for commands like "d=50"
        if (Size > 2 && uart_dma_rx_buffer[0] == 'd' && uart_dma_rx_buffer[1] == '=')
        {
            // Create a temporary buffer for the numeric part and null-terminate it
            char temp_buffer[16]; // Sufficient for "d=100" + null terminator
            uint16_t num_len = Size - 2;
            if (num_len >= sizeof(temp_buffer)) {
                num_len = sizeof(temp_buffer) - 1; // Prevent buffer overflow
            }
            memcpy(temp_buffer, &uart_dma_rx_buffer[2], num_len);
            temp_buffer[num_len] = '\0'; // Null-terminate the temporary buffer

            // Convert the numeric part to an integer
            int duty_cycle = atoi(temp_buffer);

            // Update the PWM duty cycle
            if (duty_cycle >= 0 && duty_cycle <= 100)
            {
                pwm_set_duty_cycle_percent(&htim3, PWM_TIM_CHANNEL, duty_cycle);
            }
        }

        // Restart the UART reception to wait for the next message
        UART_Receive_DMA_Start();
    }
}

/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
  }
}

/**
  * @brief  Conversion DMA half-transfer complete callback.
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  /* Set a flag to process the first half of the buffer in the main loop */
  g_adc_conv_half_cplt = true;
}

/**
  * @brief  Conversion complete callback in non-blocking mode.
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  /* Set a flag to process the second half of the buffer in the main loop */
  g_adc_conv_cplt = true;
}