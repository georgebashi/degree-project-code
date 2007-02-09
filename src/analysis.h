#ifndef ANALYSIS_H
#define ANALYSIS_H

extern float spectral_centroid(float *fft);
extern float spectral_intensity(float *fft);
extern float spectral_smoothness(float *fft);
extern float linear_regression(float *fft);
extern float zero_crossing_rate(float *fft);

int sgn(float val);

#define ANALYSIS_OUTPUTS 4
#define ZCR_SCHWARZ

#endif
