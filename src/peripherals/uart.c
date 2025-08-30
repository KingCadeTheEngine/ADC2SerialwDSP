#include "uart.h"
#include "app_config.h"
#include "gpio.h"
#include <string.h>

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;

volatile bool g_uart_tx_busy = false;
static char uart_dma_tx_buffer[UART_TX_BUFFER_SIZE]; // Dedicated buffer for UART DMA

void UART_Init(void) {
    huart2.Instance = USARTx;
    huart2.Init.BaudRate = USARTx_BAUD_RATE;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        // Initialization Error
        while(1);
    }
}

void UART_Transmit_DMA(const char* message) {
    if (!g_uart_tx_busy) {
        g_uart_tx_busy = true;
        // Copy message to dedicated DMA buffer to prevent corruption
        strncpy(uart_dma_tx_buffer, message, UART_TX_BUFFER_SIZE - 1);
        uart_dma_tx_buffer[UART_TX_BUFFER_SIZE - 1] = '\0'; // Ensure null termination
        HAL_UART_Transmit_DMA(&huart2, (uint8_t*)uart_dma_tx_buffer, strlen(uart_dma_tx_buffer));
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(huart->Instance==USART2) {
        // Peripheral clock enable
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        // USART2 GPIO Configuration
        // PA2     ------> USART2_TX
        // PA3     ------> USART2_RX
        // TX Pin
        GPIO_InitStruct.Pin = USARTx_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = USARTx_TX_AF;
        HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

        // RX Pin
        GPIO_InitStruct.Pin = USARTx_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; 
        GPIO_InitStruct.Pull = GPIO_NOPULL; 
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = USARTx_RX_AF;
        HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

        // USART2 DMA Init
        // USART2_TX Init
        USARTx_DMA_CLK_ENABLE(); // Enable DMA clock
        hdma_usart2_tx.Instance = DMA1_Stream6;
        hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
        hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart2_tx.Init.Mode = DMA_NORMAL;
        hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        HAL_DMA_Init(&hdma_usart2_tx);

        __HAL_LINKDMA(huart,hdmatx,hdma_usart2_tx);

        // Enable DMA interrupt for USART2_TX
        HAL_NVIC_EnableIRQ(USARTx_DMA_IRQn);

        // USART2 interrupt Init
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart) {
    if(huart->Instance==USART2) {
        // Peripheral clock disable
        __HAL_RCC_USART2_CLK_DISABLE();

        // USART2 GPIO Configuration
        // PA2     ------> USART2_TX
        // PA3     ------> USART2_RX
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

        // USART2 DMA DeInit
        HAL_DMA_DeInit(huart->hdmatx);

        // USART2 interrupt Deinit
        HAL_NVIC_DisableIRQ(USART2_IRQn);
    }
}
