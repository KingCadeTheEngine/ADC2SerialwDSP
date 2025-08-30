#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/*============================================================================*/
/* ADC Configuration                                                          */
/*============================================================================*/

// Associated DMA: DMA2 Stream 0, Channel 0
// Associated GPIO: PA0 (ADC1_IN0)

// ADC Peripheral
#define ADC_INSTANCE                ADC1
#define ADC_CLK_ENABLE()            __HAL_RCC_ADC1_CLK_ENABLE()

// ADC Pin Configuration
#define ADC_GPIO_PORT               GPIOA
#define ADC_GPIO_PIN                GPIO_PIN_0
#define ADC_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define ADC_CHANNEL                 ADC_CHANNEL_0

// ADC Clock and Timing
#define ADC_CLOCK_PRESCALER         ADC_CLOCK_SYNC_PCLK_DIV4
#define ADC_SAMPLING_TIME           ADC_SAMPLETIME_84CYCLES
#define ADC_RESOLUTION              ADC_RESOLUTION_12B

// ADC DMA Configuration
#define ADC_DMA_STREAM              DMA2_Stream0
#define ADC_DMA_CHANNEL             DMA_CHANNEL_0
#define ADC_DMA_CLK_ENABLE()        __HAL_RCC_DMA2_CLK_ENABLE()
#define ADC_DMA_IRQn                DMA2_Stream0_IRQn
#define ADC_DMA_IRQHandler          DMA2_Stream0_IRQHandler

// ADC Buffer
#define ADC_BUFFER_LENGTH           512

/*============================================================================*/
/* LED Blinky Configuration (using TIM)                                       */
/*============================================================================*/

// LED Pin Configuration
#define LED_GPIO_PORT               GPIOA
#define LED_GPIO_PIN                GPIO_PIN_5
#define LED_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()

/* Prescaler: 50000 -> 50MHz / 50000 = 1kHz (1ms) */
/* Period: 500 -> 1ms * 500 = 500ms interrupt */
// LED Timer Configuration
#define LED_TIM_INSTANCE            TIM2
#define LED_TIM_CLK_ENABLE()        __HAL_RCC_TIM2_CLK_ENABLE()
#define LED_TIM_PRESCALER           50000 - 1  // 50MHz / 50000 = 1kHz (1ms)
#define LED_TIM_PERIOD              500 - 1    // 1ms * 500 = 500ms interrupt


/*============================================================================*/
/* UART Configuration                                                         */
/*============================================================================*/

// Associated DMA (TX): DMA1 Stream 6, Channel 4
// Associated DMA (RX): DMA1 Stream 5, Channel 4
// Associated GPIO: PA2 (USART2_TX), PA3 (USART2_RX)

// UART Peripheral
#define USARTx                      USART2
#define USARTx_CLK_ENABLE()         __HAL_RCC_USART2_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()        __HAL_RCC_USART2_FORCE_RESET()
#define USARTx_RELEASE_RESET()      __HAL_RCC_USART2_RELEASE_RESET()

// UART Pin Configuration
#define USARTx_TX_PIN               GPIO_PIN_2
#define USARTx_TX_GPIO_PORT         GPIOA
#define USARTx_TX_AF                GPIO_AF7_USART2
#define USARTx_RX_PIN               GPIO_PIN_3
#define USARTx_RX_GPIO_PORT         GPIOA
#define USARTx_RX_AF                GPIO_AF7_USART2

// UART DMA TX Configuration
#define USARTx_TX_DMA_STREAM            DMA1_Stream6
#define USARTx_TX_DMA_CHANNEL           DMA_CHANNEL_4
#define USARTx_TX_DMA_CLK_ENABLE()      __HAL_RCC_DMA1_CLK_ENABLE()
#define USARTx_TX_DMA_IRQn              DMA1_Stream6_IRQn
#define USARTx_TX_DMA_IRQHandler        DMA1_Stream6_IRQHandler

// UART DMA RX Configuration
#define USARTx_RX_DMA_STREAM            DMA1_Stream5
#define USARTx_RX_DMA_CHANNEL           DMA_CHANNEL_4
#define USARTx_RX_DMA_CLK_ENABLE()      __HAL_RCC_DMA1_CLK_ENABLE()
#define USARTx_RX_DMA_IRQn              DMA1_Stream5_IRQn
#define USARTx_RX_DMA_IRQHandler        DMA1_Stream5_IRQHandler

// UART Baud Rate
#define USARTx_BAUD_RATE            2000000 // 2 Mbps

// UART Buffer Sizes
#define UART_TX_BUFFER_SIZE         64
#define UART_RX_BUFFER_SIZE         16


/*============================================================================*/
/* PWM Configuration                                                          */
/*============================================================================*/
// Documentation on PWM Frequency and Resolution Limitations:
//
// The achievable PWM frequency and duty cycle resolution are interdependent
// and limited by the timer's clock frequency, prescaler, and period (ARR).
//
// Frequency (f_PWM) = f_TIM_CLK / ((Prescaler + 1) * (Period + 1))
//
// Duty Cycle Resolution = 1 / (Period + 1)
//
// - Higher frequencies require smaller Period values, which reduces duty cycle resolution.
// - Lower frequencies allow larger Period values, increasing duty cycle resolution.
// - The Prescaler can be used to extend the range of achievable frequencies,
//   but also affects the effective timer clock seen by the counter.
// - For a fixed timer clock, increasing the Prescaler or Period will decrease the PWM frequency.
// - The maximum Period value is 65535 for 16-bit timers and 4294967295 for 32-bit timers.
//   (TIM3 is a 16-bit timer on STM32F446RE).
//
// When setting PWM parameters, consider the trade-off between desired frequency
// and the required granularity for duty cycle control.
//
// Current Implementation Calculations (assuming PCLK1 = 50MHz, thus TIM3_CLK = 100MHz):
//
// For PWM_DEFAULT_FREQUENCY_HZ = 1000 Hz:
//
// f_TIM_CLK = 100,000,000 Hz
// Desired f_PWM = 1,000 Hz
//
// From f_PWM = f_TIM_CLK / ((Prescaler + 1) * (Period + 1)):
// (Prescaler + 1) * (Period + 1) = f_TIM_CLK / f_PWM = 100,000,000 / 1,000 = 100,000
//
// The pwm_set_frequency function in pwm.c calculates Prescaler and Period
// to achieve the desired frequency while keeping Period within 16-bit range (max 65535).
//
// Example for 1000 Hz:
// If Prescaler = 0, Period = 99,999 (too large for 16-bit timer)
// If Prescaler = 1, Period = 49,999 (100,000 / 2 - 1) - This is used in PWM_Init for 1kHz.
//
// Duty Cycle Resolution for Period = 49999:
// Resolution = 1 / (49999 + 1) = 1 / 50000 = 0.00002 (or 0.002%)
// This means the duty cycle can be set with a granularity of 1/50000th of the period.
//

// PWM Timer Instance
#define PWM_TIM_INSTANCE            TIM3

// PWM Channel
#define PWM_TIM_CHANNEL             TIM_CHANNEL_1

// PWM GPIO Pin Configuration (PA6 for TIM3_CH1)
#define PWM_GPIO_PORT               GPIOA
#define PWM_GPIO_PIN                GPIO_PIN_6
#define PWM_GPIO_AF                 GPIO_AF2_TIM3 // Alternate Function for TIM3_CH1 on PA6

// Default PWM Parameters
#define PWM_DEFAULT_FREQUENCY_HZ    1000    // Default frequency in Hz (e.g., 1 kHz)
#define PWM_DEFAULT_DUTY_CYCLE_PERCENT 75   // Default duty cycle in percentage (0-100)


#endif /* __APP_CONFIG_H */