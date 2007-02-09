#ifndef ANALYSIS_H
#define ANALYSIS_H

extern float spectral_centroid(float *fft);
extern float spectral_intensity(float *fft);
extern float spectral_smoothness(float *fft);
extern float linear_regression(float *fft);
extern float zero_crossing_rate(float *signal);
extern float first_order_autocorrelation(float *signal);
extern float spectral_spread(float spectral_centroid, float *fft);
extern float spectral_dissymmetry(float spectral_centroid, float *fft);
int sgn(float val);

#define ANALYSIS_OUTPUTS 8
#define ANALYSIS_OUTPUT_CSV_HEADER \
    "\"Intensity\",\"Centroid\",\"Smoothness\",\"Linear Regression (gradient)\",\"Zero Crossing Rate\",\"First Order Autocorrelation\",\"Spectral Spread\",\"Spectral Dissymmetry\""

#define ZCR_SCHWARZ

#endif
