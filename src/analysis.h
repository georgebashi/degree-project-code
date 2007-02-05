#ifndef ANALYSIS_H
#define ANALYSIS_H

extern float spectral_centroid(float *fft);
extern float spectral_intensity(float *fft);
extern float spectral_smoothness(float *fft);
extern float linear_regression(float *fft);

extern void analysis(float *fft, float* outputs);

#define ANALYSIS_OUTPUTS 4

#endif
