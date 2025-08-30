#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#include <stdbool.h> // Include for bool type

extern volatile bool g_uart_error_flag; // Declare global error flag

#ifdef __cplusplus
 extern "C" {
#endif

void SysTick_Handler(void);
void TIM2_IRQHandler(void);
void ADC_DMA_IRQHandler(void);
void USARTx_TX_DMA_IRQHandler(void); // Renamed for clarity
void USARTx_RX_DMA_IRQHandler(void); // Added for RX
void USART2_IRQHandler(void);


#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_IT_H */