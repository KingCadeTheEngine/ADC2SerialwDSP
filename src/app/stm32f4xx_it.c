#include "main.h"
#include "stm32f4xx_it.h"
#include "tim.h"
#include "adc.h"
#include "uart.h"

/* USER CODE BEGIN Includes */
#include "app_logic.h"
/* USER CODE END Includes */

extern UART_HandleTypeDef huart2;
extern volatile bool g_uart_tx_busy;

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
  * @brief This function handles DMA1 stream6 global interrupt.
  */
void USARTx_DMA_IRQHandler(void)
{
  HAL_DMA_IRQHandler(huart2.hdmatx);
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
  // This callback is called when the UART DMA transmission is complete.
  // You can add any post-transmission logic here.
  // For now, just having it defined is enough to allow the HAL to clear the busy state.
  if(huart->Instance == USART2)
  {
    g_uart_tx_busy = false;
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