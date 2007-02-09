
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "common.h"
#include "analysis.h"

float spectral_centroid(float *fft) {
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

float spectral_intensity(float *fft) {
    return fft[0];
}

float spectral_smoothness(float *fft) {
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

/*

LINEAR REGRESSION
x = i, y = fft_output[i]
b = gradient, a = intercept

Pete: the take sx=sum of x's, sy=sum of ys, sxx=sum of x squared, syy=sum of y sqared, sxy=sum of xy's
Pete: then b which is the slope can be estimated by (nsxy-sxsy)/(nsxx-sxsx)
Pete: if u get dat?
Pete: where n is obv the number of points

a=(sy-b*sx)/n

*/
float linear_regression(float *fft) {
    int i;
    float sx = 5625088.0f, sxx = 82480665344.0f, sy = 0, syy = 0, sxy = 0;
    for (i = 1; i < BINS; i++) {
        sy += fft[i];
        syy += pow(fft[i], 2);
        sxy += i * 43 * fft[i];
    }
    return ((BINS * sxy) - (sx * sy)) / ((BINS * sxx) - pow(sx, 2));
}

float zero_crossing_rate(float *signal) {
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


