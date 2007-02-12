#ifndef ANALYSIS_H
#define ANALYSIS_H

#define ZCR_SCHWARZ

#define ANALYSIS_OUTPUTS 8
#define ANALYSIS_OUTPUT_CSV_HEADER \
    "\"Zero Crossing Rate\",\"First Order Autocorrelation\",\"Linear Regression (gradient)\",\"Centroid\",\"Intensity\",\"Smoothness\",\"Spread\",\"Dissymmetry\""
#define ZERO_CROSSING_RATE 0
#define FIRST_ORDER_AUTOCORRELATION 1
#define LINEAR_REGRESSION 2
#define SPECTRAL_CENTROID 3
#define SPECTRAL_INTENSITY 4
#define SPECTRAL_SMOOTHNESS 5
#define SPECTRAL_SPREAD 6
#define SPECTRAL_DISSYMMETRY 7

extern float zero_crossing_rate(float *signal);
extern float first_order_autocorrelation(float *signal);
extern float linear_regression(float *fft);
extern float spectral_centroid(float *fft);
extern float spectral_intensity(float *fft);
extern float spectral_smoothness(float *fft);
extern float spectral_spread(float spectral_centroid, float *fft);
extern float spectral_dissymmetry(float spectral_centroid, float *fft);

extern float * extract_features(float *window, float *fft_output);

#endif
