#include <cmath>
#include <string>
#include <fftw3.h>

#include "common.hh"
#include "FFT.hh"

FFT::FFT(float *out)
{
    // create and store hanning window
    hanning_window = new float[WINDOW_SIZE];
    fft_input = (float *) fftwf_malloc(sizeof(float) * WINDOW_SIZE);
    float ang = (1.0 / WINDOW_SIZE) * TWOPI;
    
    for (int i = 0; i < WINDOW_SIZE; i++) {
        hanning_window[i] = 0.5 - 0.5 * cos(ang * i);
    }
    FILE* wisdom;
    if ((wisdom = fopen("/home/george/.extractor/wisdom", "r")) != NULL) {
        // fftw leaks memory malloc'd in this call...
        fftwf_import_wisdom_from_file(wisdom);
        fclose(wisdom);
        plan = fftwf_plan_r2r_1d(WINDOW_SIZE, fft_input, out, FFTW_R2HC, FFTW_EXHAUSTIVE);
    } else {
        plan = fftwf_plan_r2r_1d(WINDOW_SIZE, fft_input, out, FFTW_R2HC, FFTW_EXHAUSTIVE);
        wisdom = fopen("/home/george/.extractor/wisdom", "w");
        if (wisdom != NULL) {
            fftwf_export_wisdom_to_file(wisdom);
            fclose(wisdom);
        }
    }
}

FFT::~FFT()
{
    fftwf_free(fft_input);
    delete [] hanning_window;
    fftwf_forget_wisdom();
    fftwf_destroy_plan(plan);
}

void FFT::do_window(float *in)
{
    memcpy(fft_input, in, WINDOW_SIZE * sizeof(float));
    
    for (int i = 0; i < WINDOW_SIZE; i++) {
        fft_input[i] *= hanning_window[i];
    }
    
    fftwf_execute(plan);
}

void FFT::fix_output(float *output)
{
    // fix power
    output[0] *= output[0];
    for (int i = 1; i < BINS; i++) {
        output[i] = pow(output[i], 2) + pow(output[WINDOW_SIZE - i], 2);
    }
}

