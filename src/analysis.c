
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


void analysis(float *fft, float *outputs) {
    int i;
    float sx = 5625088.0f, sxx = 82480665344.0f, sy = 0, syy = 0, sxy = 0, ss = 0, total_amp = 0, bins = 0;
    const int last_bin = BINS - 1;
    float current_amp;
    for (i = 1; i < BINS; i++) {
        current_amp = fft[i];
        
        bins += i * current_amp;  // sc
        total_amp += current_amp;
        
        if (i > 1 && i < last_bin) { // ss
            ss += (20 * log(current_amp)) - ((
                (20 * logf(fft[i - 1])) +
                (20 * logf(current_amp)) +
                (20 * logf(fft[i + 1]))
            ) / 3);
        }
        
        sy += current_amp;
        syy += pow(current_amp, 2);
        sxy += i * 43 * current_amp;
        
        __builtin_prefetch(&fft[i+1], 0, 1);
        __builtin_prefetch(&fft[i+2], 0, 1);
    }
    outputs[0] = fft[0]; // si
    outputs[1] = bins / total_amp; // sc
    outputs[2] = ss;
    outputs[3] = ((BINS * sxy) - (sx * sy)) / ((BINS * sxx) - pow(sx, 2));
}

