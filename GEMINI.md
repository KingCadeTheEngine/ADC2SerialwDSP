# Project: NucleoTest3 - STM32F446RE Embedded Application

## Overview

This project is an embedded application for the STM32 Nucleo-F446RE development board, built using the PlatformIO ecosystem and the STM32Cube HAL framework. It initializes GPIO, a timer (TIM2), and an ADC (ADC1) with DMA, processing ADC data from a DMA-filled buffer.

### Hardware:
*   **Board:** Nucleo-F446RE
*   **Microcontroller:** STM32F446RE

### Software:
*   **IDE:** PlatformIO
*   **Framework:** STM32Cube HAL
*   **Language:** C
*   **SDK:** STMicroelectronics STM32Cube

## Project Structure and Important Files

The source code in `src/` is organized into logical subdirectories for clarity and modularity.

*   `src/`: Organized source code.
    *   `app/`: High-level application logic.
        *   `app_logic.c/.h`: Contains `app_setup()` and `app_loop()`.
        *   `app_config.h`: Application-specific configuration constants and hardware definitions.
        *   `stm32f4xx_it.c/.h`: Interrupt service routines and their callbacks.
        *   `dsp_utils.c/.h`: Digital Signal Processing utilities, including Goertzel algorithm.
    *   `core/`: Core system initialization and main entry point.
        *   `main.c/.h`: Program entry point and global error handler.
        *   `system_config.c/.h`: Low-level system and clock configuration.
    *   `peripherals/`: Low-level drivers and initialization for MCU peripherals.
        *   `adc.c/.h`, `gpio.c/.h`, `tim.c/.h`, `uart.c/.h`: Peripheral configuration and initialization code.
*   `platformio.ini`: PlatformIO project configuration file, including `build_flags` for `src/` directory structure.

## Quick Start

*   **Build:** `pio run`
*   **Upload:** `pio run --target upload`
*   **Clean:** `pio run --target clean`
*   **Monitor:** `pio device monitor`

## Coding Style

Emphasizes clean, modular code for easy understanding.

*   **Arduino-like Structure:** `app_setup()` and `app_loop()` abstract application logic.
*   **Directory Organization:** `src` is organized by functional layers (`app`, `core`, `peripherals`).
*   **Configuration via Headers:** Hardware and application settings are defined as constants in `app_config.h`.
*   **Header Co-location:** Header files (`.h`) are kept with their corresponding source files (`.c`).

## Key Learnings and Project Changes

This section summarizes key insights gained during a debugging session and the resulting project modifications, particularly concerning UART DMA communication and DSP implementation.

### Project Changes:

*   **`src/app/stm32f4xx_it.c`**: 
    *   Added `SysTick_Handler` (calls `HAL_IncTick()`) to enable the HAL's internal timekeeping.
    *   Added `HAL_UART_TxCpltCallback` (empty implementation) to properly manage UART DMA transfer completion.
    *   Added `USARTx_RX_DMA_IRQHandler()` for the RX DMA stream.
    *   Implemented `HAL_UARTEx_RxEventCallback()` to parse incoming commands (e.g., "d=50") and update PWM duty cycle using `pwm_set_duty_cycle_percent()`.
*   **`src/app/stm32f4xx_it.h`**: 
    *   Declared `SysTick_Handler`.
    *   Declared `USARTx_RX_DMA_IRQHandler()`.
*   **`src/peripherals/uart.c`**: 
    *   Switched `UART_Transmit_DMA` to use `HAL_UART_Transmit_DMA` for efficient data transfer.
    *   Enabled the DMA interrupt for the UART transmit stream (`HAL_NVIC_EnableIRQ(USARTx_DMA_IRQn);`) in `HAL_UART_MspInit`.
    *   Enabled `UART_MODE_TX_RX` in `UART_Init()`.
    *   Configured DMA for UART RX (`DMA1_Stream5`, Channel 4) in `HAL_UART_MspInit()`.
    *   Implemented `UART_Receive_DMA_Start()` using `HAL_UARTEx_ReceiveToIdle_DMA()`.
    *   Updated `UART_Transmit_DMA()` function signature and implementation for robustness.
*   **`src/peripherals/uart.h`**: 
    *   Declared `hdma_usart2_rx`, `uart_dma_rx_buffer`, and `UART_Receive_DMA_Start()`.
*   **`src/app/app_config.h`**: 
    *   Added definitions for UART RX DMA stream, channel, and buffer size (`UART_RX_BUFFER_SIZE`).
    *   **New:** Added `DSP_WINDOW_LENGTH` (64) for DSP window size configuration.
*   **`src/app/app_logic.c`**: 
    *   Made `htim3` (PWM timer handle) global.
    *   Called `UART_Receive_DMA_Start()` in `app_setup()`.
    *   Updated `UART_Transmit_DMA()` calls to match the new signature.
    *   **New:** Removed `process_adc_data` function.
    *   **New:** Integrated DSP processing into `app_loop()`: processes ADC DMA halves in 64-sample chunks using `DSP_ProcessWindow()`, formats results, and sends via `UART_Transmit_DMA()`. Removed tick-based rate limiting.
    *   **New:** Replaced literal `64` with `DSP_WINDOW_LENGTH` for window size references.
*   **`src/app/app_logic.h`**: 
    *   Declared `htim3` as `extern`.
    *   **New:** Included `dsp_utils.h` and `app_config.h`.
    *   **New:** Declared `extern` for `ADC_HandleTypeDef hadc1;` and `UART_HandleTypeDef huart2;`.
    *   **New:** Declared `extern` for `g_adc_dma_buf`, `g_adc_conv_half_cplt`, and `g_adc_conv_cplt`.
*   **New Module: `src/app/dsp_utils.c/.h`**: 
    *   Implemented the Goertzel algorithm for efficient single-frequency analysis (39kHz, 40kHz, 41kHz).
    *   `DSP_Init()`: Initializes the DSP module, including pre-calculating Goertzel coefficients.
    *   `DSP_ProcessWindow()`: Processes a window of ADC data, calculates magnitude and phase for target frequencies.
    *   Included `app_config.h` and ensured `M_PI` definition.

### Learnings for Gemini Agents:

*   **SysTick is Fundamental:** The `SysTick_Handler` and `uwTick` counter are critical for STM32Cube HAL's internal timing, timeouts, and state management. Their absence or misconfiguration can cause system hangs, even in functions not explicitly using `HAL_Delay()`).
*   **HAL Dependencies:** Many HAL functions rely on other HAL components (e.g., DMA interrupt handlers, callbacks). A problem in one area can manifest as a failure in another seemingly unrelated area.
*   **Systematic Debugging:** When facing low-level issues, a systematic approach is essential. This includes:
    *   Verifying basic functionality (clocks, GPIO) early.
    *   Inspecting register values directly.
    *   Utilizing debuggers for precise diagnosis.
*   **Working with the User:**
    *   **Leverage User's Debugging Capabilities:** When the user states they have a debugger, immediately pivot to using it for precise diagnosis. Direct debugger inspection is often more efficient than indirect methods.
    *   **User's Domain Knowledge is Invaluable:** Always listen carefully to user feedback, especially their corrections and observations. Their direct experience with the hardware and code can significantly accelerate the debugging process. Be prepared to adjust your understanding based on their insights.
*   **Efficient UART Reception:** For non-blocking, variable-length message reception, combining DMA with the UART's Idle Line detection (`HAL_UARTEx_ReceiveToIdle_DMA`) is highly efficient. It offloads byte-by-byte transfer to DMA and only interrupts the CPU once per message, when the transmission is complete. This is superior to byte-by-byte interrupt handling for message-based communication.
