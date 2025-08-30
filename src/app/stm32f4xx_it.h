#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

void SysTick_Handler(void);
void TIM2_IRQHandler(void);
void ADC_DMA_IRQHandler(void);
void USARTx_DMA_IRQHandler(void);
void USART2_IRQHandler(void);


#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_IT_H */