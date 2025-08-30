#ifndef __DSP_UTILS_H
#define __DSP_UTILS_H

#include <stdint.h>

// Define a struct to hold the Goertzel results for each frequency
typedef struct {
    float magnitude_39khz;
    float phase_39khz;
    float magnitude_40khz;
    float phase_40khz;
    float magnitude_41khz;
    float phase_41khz;
} GoertzelResults_t;

// Function to initialize the DSP module (e.g., pre-calculate coefficients)
void DSP_Init(void);

// Function to process a window of ADC data using the Goertzel algorithm
// Returns 1 if successful, 0 if an error occurred (e.g., invalid window size)
uint8_t DSP_ProcessWindow(const uint16_t *adc_data_buffer, uint32_t window_size, GoertzelResults_t *results);

#endif // __DSP_UTILS_H
