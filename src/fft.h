#ifndef FFT_H
#define FFT_H

extern fftwf_plan fft_setup(float *out);
extern void fft_stop();
extern void fft_do_window(fftwf_plan *plan, float *in);
extern void fft_fix_output(float *output);


// globals
float *hanning_window;
float *fft_input;

#endif
