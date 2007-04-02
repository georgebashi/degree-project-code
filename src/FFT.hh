// $Id$

#ifndef FFT_HH
#define FFT_HH

#include <fftw3.h>

class FFT
{
    public:
        FFT(float *out);
        ~FFT();
        void do_window(float *in);
        void fix_output(float *output);
    private:
        fftwf_plan plan;
        float *hanning_window;
        float *fft_input;
};

#endif

