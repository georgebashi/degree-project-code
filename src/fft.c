#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fftw3.h>

#include "common.h"
#include "fft.h"
#include "extractor.h"
#include "output.h"

// bins are +43 each time
// 22050/512

fftwf_plan fft_setup(float *out)
{
    // create and store hanning window
    hanning_window = malloc(sizeof(float) * WINDOW_SIZE);
    fft_input = fftwf_malloc(sizeof(float) * WINDOW_SIZE);
    float ang = (1.0 / WINDOW_SIZE) * TWOPI;
    int i;
    for(i = 0; i < WINDOW_SIZE; i++)
    {
        hanning_window[i] = 0.5 - 0.5 * cos(ang * i);
    }
    FILE * wisdom;
    if ((wisdom = fopen(".wisdom", "r")) != NULL && !reprofile) {
        fftwf_import_wisdom_from_file(wisdom);
        fclose(wisdom);
        return fftwf_plan_r2r_1d(WINDOW_SIZE, fft_input, out, FFTW_R2HC, FFTW_EXHAUSTIVE);
    } else {
        fftwf_plan plan = fftwf_plan_r2r_1d(WINDOW_SIZE, fft_input, out, FFTW_R2HC, FFTW_EXHAUSTIVE);
        wisdom = fopen(".wisdom", "w");
        if (wisdom != NULL) {
            fftwf_export_wisdom_to_file(wisdom);
            fclose(wisdom);
        }
        return plan;
    }
}

void fft_stop() {
    fftwf_free(fft_input);
    free(hanning_window);
}

void fft_do_window(fftwf_plan *plan, float *in)
{
    memcpy(fft_input, in, WINDOW_SIZE * sizeof(float));
    int i;
    for (i = 0; i < WINDOW_SIZE; i++)
    {
        fft_input[i] *= hanning_window[i];
    }
    fftwf_execute(*plan);
    
    if (print_data == OPT_PRINT_WINDOW) {
        int i;
        for (i = 0; i < WINDOW_SIZE; i++) {
            print_csv_data(stdout, 1, "\"FFT Input (first window)\"", fft_input[i]);
        }
        exit(0);
    }
}

void fft_fix_output(float *output) {
    int i;
    // fix power
    output[0] *= output[0];
    for (i = 1; i < BINS; i++) {
        output[i] = pow(output[i], 2) + pow(output[WINDOW_SIZE - i], 2);
    }
}
