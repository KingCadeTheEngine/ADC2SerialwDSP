#include "dsp_utils.h"
#include <math.h>
#include "app_config.h" // Include for DSP_WINDOW_LENGTH

#ifndef M_PI
#define M_PI 3.14159265359f
#endif

// Define the ADC sampling rate (Fs) and target frequencies
#define F_SAMPLING_HZ       260417.0f // Approximately 260.417 kHz
#define F_TARGET_39KHZ      39000.0f
#define F_TARGET_40KHZ      40000.0f
#define F_TARGET_41KHZ      41000.0f

// Pre-calculated Goertzel coefficients
static float coeff_39khz;
static float coeff_40khz;
static float coeff_41khz;

// Function to calculate Goertzel coefficient
static float calculate_goertzel_coeff(float target_freq, uint32_t window_size) {
    float k = (target_freq * window_size) / F_SAMPLING_HZ;
    return 2.0f * cosf(2.0f * M_PI * k / window_size);
}

// Goertzel algorithm implementation for a single frequency
// Returns magnitude and phase
static void goertzel_process(const uint16_t *adc_data, uint32_t window_size, float coeff, float *magnitude, float *phase) {
    float s0 = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;

    for (uint32_t i = 0; i < window_size; i++) {
        // Normalize ADC value (assuming 12-bit ADC, 0-4095 range)
        // Adjust this normalization based on your actual ADC voltage reference and input range
        float sample = (float)adc_data[i] / 4095.0f; 
        s0 = sample + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    // Calculate magnitude squared
    float real = s1 - s2 * coeff / 2.0f;
    float imag = s2 * sinf(acosf(coeff / 2.0f));

    *magnitude = sqrtf(real * real + imag * imag);
    *phase = atan2f(imag, real);
}

void DSP_Init(void) {
    // Coefficients are calculated based on a fixed window size (e.g., 64 samples)
    // If window size can change, these need to be recalculated or passed dynamically.
    // For now, assuming a fixed window size of 64 as discussed.
    coeff_39khz = calculate_goertzel_coeff(F_TARGET_39KHZ, DSP_WINDOW_LENGTH);
    coeff_40khz = calculate_goertzel_coeff(F_TARGET_40KHZ, DSP_WINDOW_LENGTH);
    coeff_41khz = calculate_goertzel_coeff(F_TARGET_41KHZ, DSP_WINDOW_LENGTH);
}

uint8_t DSP_ProcessWindow(const uint16_t *adc_data_buffer, uint32_t window_size, GoertzelResults_t *results) {
    if (adc_data_buffer == NULL || results == NULL || window_size == 0) {
        return 0; // Invalid input
    }
    if (window_size != DSP_WINDOW_LENGTH) {
        // For now, we assume a fixed window size of 64 for pre-calculated coefficients
        // If dynamic window sizes are needed, coefficients must be recalculated here.
        return 0; 
    }

    // Process for 39kHz
    goertzel_process(adc_data_buffer, window_size, coeff_39khz, &results->magnitude_39khz, &results->phase_39khz);

    // Process for 40kHz
    goertzel_process(adc_data_buffer, window_size, coeff_40khz, &results->magnitude_40khz, &results->phase_40khz);

    // Process for 41kHz
    goertzel_process(adc_data_buffer, window_size, coeff_41khz, &results->magnitude_41khz, &results->phase_41khz);

    return 1; // Success
}
