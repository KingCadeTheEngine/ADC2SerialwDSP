#include "uart.h"
#include "app_config.h"
#include "gpio.h"
#include <string.h>

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx; // RX DMA Handle

volatile bool g_uart_tx_busy = false;
static char uart_dma_tx_buffer[UART_TX_BUFFER_SIZE]; // Dedicated buffer for UART DMA TX
uint8_t uart_dma_rx_buffer[UART_RX_BUFFER_SIZE];     // Dedicated buffer for UART DMA RX

void UART_Init(void) {
    huart2.Instance = USARTx;
    huart2.Init.BaudRate = USARTx_BAUD_RATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX; // Enable both TX and RX
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        // Initialization Error
        while(1);
    }
}

void UART_Transmit_DMA(const uint8_t* data, uint16_t size) {
    if (!g_uart_tx_busy && size > 0 && size < UART_TX_BUFFER_SIZE) {
        g_uart_tx_busy = true;
        // Copy message to dedicated DMA buffer to prevent corruption from calling scope
        memcpy(uart_dma_tx_buffer, data, size);
        HAL_UART_Transmit_DMA(&huart2, (uint8_t*)uart_dma_tx_buffer, size);
    }
}

void UART_Receive_DMA_Start(void) {
    // Safely stop DMA before restarting reception
    HAL_UART_DMAStop(&huart2);
    // Start UART reception in DMA mode with Idle Line detection
    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_dma_rx_buffer, UART_RX_BUFFER_SIZE) != HAL_OK) {
        // Reception start error
        while(1);
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(huart->Instance==USART2) {
        // Peripheral clock enable
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        USARTx_TX_DMA_CLK_ENABLE(); // Enable DMA clock for TX
        USARTx_RX_DMA_CLK_ENABLE(); // Enable DMA clock for RX

        // USART2 GPIO Configuration
        // PA2     ------> USART2_TX
        // PA3     ------> USART2_RX
        GPIO_InitStruct.Pin = USARTx_TX_PIN | USARTx_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = USARTx_TX_AF; // Same AF for both pins
        HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

        /* USART2 DMA Init */

        // USART2_TX Init (DMA1_Stream6, Channel 4)
        hdma_usart2_tx.Instance = USARTx_TX_DMA_STREAM;
        hdma_usart2_tx.Init.Channel = USARTx_TX_DMA_CHANNEL;
        hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart2_tx.Init.Mode = DMA_NORMAL;
        hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        HAL_DMA_Init(&hdma_usart2_tx);
        __HAL_LINKDMA(huart, hdmatx, hdma_usart2_tx);

        // USART2_RX Init (DMA1_Stream5, Channel 4)
        hdma_usart2_rx.Instance = USARTx_RX_DMA_STREAM;
        hdma_usart2_rx.Init.Channel = USARTx_RX_DMA_CHANNEL;
        hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart2_rx.Init.Mode = DMA_NORMAL;
        hdma_usart2_rx.Init.Priority = DMA_PRIORITY_HIGH; // Higher priority for RX
        hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        HAL_DMA_Init(&hdma_usart2_rx);
        __HAL_LINKDMA(huart, hdmarx, hdma_usart2_rx);

        /* DMA interrupt init */
        // TX DMA interrupt
        HAL_NVIC_SetPriority(USARTx_TX_DMA_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USARTx_TX_DMA_IRQn);
        // RX DMA interrupt
        HAL_NVIC_SetPriority(USARTx_RX_DMA_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USARTx_RX_DMA_IRQn);

        // USART2 interrupt Init
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart) {
    if(huart->Instance==USART2) {
        // Peripheral clock disable
        __HAL_RCC_USART2_CLK_DISABLE();

        // USART2 GPIO DeConfiguration
        HAL_GPIO_DeInit(GPIOA, USARTx_TX_PIN | USARTx_RX_PIN);

        // USART2 DMA DeInit
        HAL_DMA_DeInit(huart->hdmatx);
        HAL_DMA_DeInit(huart->hdmarx);

        // Interrupt Deinit
        HAL_NVIC_DisableIRQ(USART2_IRQn);
        HAL_NVIC_DisableIRQ(USARTx_TX_DMA_IRQn);
        HAL_NVIC_DisableIRQ(USARTx_RX_DMA_IRQn);
    }
}
