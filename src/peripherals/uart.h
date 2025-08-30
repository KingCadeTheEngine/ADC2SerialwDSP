#ifndef UART_H
#define UART_H

#include "stm32f4xx_hal.h"
#include "app_config.h"
#include <stdbool.h>

// HAL Handles
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;

// Status Flags
extern volatile bool g_uart_tx_busy;

// Buffers
extern uint8_t uart_dma_rx_buffer[UART_RX_BUFFER_SIZE];

// Public Functions
void UART_Init(void);
void UART_Transmit_DMA(const uint8_t* data, uint16_t size);
void UART_Receive_DMA_Start(void);

#endif // UART_H
