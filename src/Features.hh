// $Id$

#ifndef FEATURES_H
#define FEATURES_H

#define NUMBER_OF_FEATURES 8

#define ZERO_CROSSING_RATE 0
#define FIRST_ORDER_AUTOCORRELATION 1
#define LINEAR_REGRESSION 2
#define SPECTRAL_CENTROID 3
#define SPECTRAL_INTENSITY 4
#define SPECTRAL_SMOOTHNESS 5
#define SPECTRAL_SPREAD 6
#define SPECTRAL_DISSYMMETRY 7

#define MEAN 0
#define VARIANCE 1
#define SKEWNESS 2
#define KURTOSIS 3
#define NUMBER_OF_AGGREGATE_STATS 4

#define ZCR_SCHWARZ

#define SONG_FEATURE_WEIGHT 1
#define BLOCK_FEATURE_WEIGHT 1

#include <vector>

class FeatureSet
{
    public:
        float* features;
        FeatureSet(float** features);
        ~FeatureSet();
};

class FeatureGroup
{
    public:
        FeatureGroup() {};
        FeatureGroup(std::vector<FeatureSet *>* sets);
        float mean[NUMBER_OF_FEATURES];
        float variance[NUMBER_OF_FEATURES];
        float skewness[NUMBER_OF_FEATURES];
        float kurtosis[NUMBER_OF_FEATURES];
};

float get_mean(float *data, int n);
float get_variance(float *data, float mean, int n);
float get_stdev(float variance);
float get_skewness(float *data, float mean, float stdev, int n);
float get_kurtosis(float *data, float mean, float stdev, int n);
inline float check_nan(float n);

class FeatureExtractor
{
    public:
        FeatureSet* process(float *signal, float* fft);
    private:
        float zero_crossing_rate(float *signal);
        float first_order_autocorrelation(float *signal);
        float linear_regression(float *fft);
        float spectral_centroid(float *fft);
        float spectral_intensity(float *fft);
        float spectral_smoothness(float *fft);
        float spectral_spread(float spectral_centroid, float *fft);
        float spectral_dissymmetry(float spectral_centroid, float *fft);
};



#endif
