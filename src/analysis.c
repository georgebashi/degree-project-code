#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "common.h"
#include "analysis.h"

float zero_crossing_rate(float *signal)
{
    int total = 0, i = 0;
    for (i = 0; i < WINDOW_SIZE - 1; i++) {
#ifdef ZCR_SCHWARZ
        total += (signal[i] < 0 ? 1 : 0) ^ (signal[i + 1] < 0 ? 1 : 0);
#else
        
        total += ((signal[i] * signal[i + 1]) < 0);
#endif
        
    }
    return total * (1.0f / WINDOW_SIZE);
}

float first_order_autocorrelation(float *signal)
{
    float total = 0;
    int i = 0;
    for (i = 0; i < WINDOW_SIZE - 1; i++) {
        total += signal[i] * signal[i + 1];
    }
    return total * (1.0f / WINDOW_SIZE);
}

float linear_regression(float *fft)
{
    int i;
    float sx = 5625088.0f, sxx = 82480665344.0f, sy = 0, syy = 0, sxy = 0;
    for (i = 1; i < BINS; i++) {
        sy += fft[i];
        syy += pow(fft[i], 2);
        sxy += i * 43 * fft[i];
    }
    return ((BINS * sxy) - (sx * sy)) / ((BINS * sxx) - pow(sx, 2));
}

float spectral_centroid(float *fft)
{
    // skip bin 0
    // bin_number * bin_amp / amps
    int i;
    float total_amp = 0;
    float bins = 0;
    for (i = 1; i < BINS; i++) {
        bins += i * fft[i];
        total_amp += fft[i];
    }
    return bins / total_amp;
}

float spectral_intensity(float *fft)
{
    return fft[0];
}

float spectral_smoothness(float *fft)
{
    int i;
    float ss = 0;
    for (i = 2; i < BINS - 1; i++) {
        ss += (20 * log(fft[i])) - ((
                                        (20 * logf(fft[i - 1])) +
                                        (20 * logf(fft[i])) +
                                        (20 * logf(fft[i + 1]))
                                    ) / 3);
    }
    return fabsf(ss);
}

float spectral_spread(float spectral_centroid, float *fft)
{
    float top = 0, bottom = 0;
    int i = 0;
    for (i = 0; i < WINDOW_SIZE; i++) {
        top += fft[i] * pow((((float)i / BINS) * 22100) - spectral_centroid, 2);
        bottom += fft[i];
    }
    return sqrtf(top / bottom);
}

float spectral_dissymmetry(float spectral_centroid, float *fft)
{
    float top = 0, bottom = 0;
    int i = 0;
    for (i = 0; i < WINDOW_SIZE; i++) {
        top += fft[i] * pow((((float)i / BINS) * 22100) - spectral_centroid, 3);
        bottom += fft[i];
    }
    return cbrtf(top / bottom);
}

float * extract_features(float *window, float *fft_output)
{
    float *features = malloc(sizeof(float) * ANALYSIS_OUTPUTS);
    features[ZERO_CROSSING_RATE] = zero_crossing_rate(window);
    features[FIRST_ORDER_AUTOCORRELATION] = first_order_autocorrelation(window);
    features[LINEAR_REGRESSION] = linear_regression(fft_output);
    features[SPECTRAL_CENTROID] = spectral_centroid(fft_output);
    features[SPECTRAL_INTENSITY] = spectral_intensity(fft_output);
    features[SPECTRAL_SMOOTHNESS] = spectral_smoothness(fft_output);
    features[SPECTRAL_SPREAD] = spectral_spread(features[SPECTRAL_CENTROID], fft_output);
    features[SPECTRAL_DISSYMMETRY] = spectral_dissymmetry(features[SPECTRAL_CENTROID], fft_output);
    
    return features;
}
