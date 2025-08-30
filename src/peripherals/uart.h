#ifndef UART_H
#define UART_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

extern volatile bool g_uart_tx_busy;

void UART_Init(void);
void UART_Transmit_DMA(const char* message);

#endif // UART_H
