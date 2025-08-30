#include "adc.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC_INSTANCE;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_PRESCALER;
  hadc1.Init.Resolution = ADC_RESOLUTION;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLING_TIME;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
* @brief ADC MSP Initialization
* This function configures the hardware resources needed for ADC:
* - Peripheral's clock enable
* - GPIO Pin configuration
* - DMA configuration
* - NVIC configuration
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC_INSTANCE)
  {
    /* Peripheral clock enable */
    ADC_CLK_ENABLE();
    ADC_DMA_CLK_ENABLE();

    /* ADC1 DMA Init */
    /* ADC1 is on DMA2 Stream 0 Channel 0 */
    hdma_adc1.Instance = ADC_DMA_STREAM;
    hdma_adc1.Init.Channel = ADC_DMA_CHANNEL;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc1);

    /* DMA2_Stream0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(ADC_DMA_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_DMA_IRQn);
  }
}