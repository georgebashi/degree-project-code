// $Id$

#include <cmath>
#include <vector>

#include "common.hh"
#include "Song.hh"
#include "Features.hh"

FeatureSet::FeatureSet(float** features)
{
    this->features = *features;
}
FeatureSet::~FeatureSet()
{
    delete[] features;
}

FeatureGroup::FeatureGroup(std::vector<FeatureSet *>* sets)
{
    for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        unsigned int data_points = sets->size();
        
        float data[data_points];
        
        for (unsigned int i = 0; i < data_points; i++) {
            data[i] = sets->at(i)->features[feature];
        }
        features[feature][MEAN] = check_nan(get_mean(data, data_points));
        features[feature][VARIANCE] = check_nan(get_variance(data, features[feature][MEAN], data_points));
        float stdev = check_nan(get_stdev(features[feature][VARIANCE]));
        features[feature][SKEWNESS] = check_nan(get_skewness(data, features[feature][MEAN], stdev, data_points));
        features[feature][KURTOSIS] = check_nan(get_kurtosis(data, features[feature][MEAN], stdev, data_points));
    }
}

FeatureGroup::FeatureGroup(FeatureGroup* fg1, FeatureGroup* fg2, float dist)
{
    for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
            this->features[feature][stat] = fg1->features[feature][stat] + (dist * (fg2->features[feature][stat] - fg1->features[feature][stat]));
        }
    }
}

float FeatureGroup::compare(FeatureGroup* other, float (*weights)[NUMBER_OF_AGGREGATE_STATS])
{
    float diff = 0;
    for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
            diff += fabsf(features[feature][stat] - other->features[feature][stat]) * weights[feature][stat];
        }
    }
    return diff;
}

float get_mean(float *data, int n)
{
    float total = 0;
    for (int i = 0; i < n; i++) {
        total += data[i];
    }
    return total / n;
}
float get_variance(float *data, float mean, int n)
{
    float total = 0;
    for (int i = 0; i < n; i++) {
        total += powf(data[i] - mean, 2);
    }
    return total / (n - 1);
}
float get_stdev(float variance)
{
    return sqrtf(variance);
}
float get_skewness(float *data, float mean, float stdev, int n)
{
    float total = 0;
    for (int i = 0; i < n; i++) {
        total += powf(data[i] - mean, 3);
    }
    return total / (n * pow(stdev, 3));
}
float get_kurtosis(float *data, float mean, float stdev, int n)
{
    float total = 0;
    for (int i = 0; i < n; i++) {
        total += powf(data[i] - mean, 4);
    }
    return total / (WINDOWS_PER_BLOCK * pow(stdev, 4));
}

FeatureSet* FeatureExtractor::process(float* signal, float* fft)
{
    float* features = new float[NUMBER_OF_FEATURES];
    
    features[ZERO_CROSSING_RATE] = check_nan(zero_crossing_rate(signal));
    features[FIRST_ORDER_AUTOCORRELATION] = check_nan(first_order_autocorrelation(signal));
    features[LINEAR_REGRESSION] = check_nan(linear_regression(fft));
    features[SPECTRAL_CENTROID] = check_nan(spectral_centroid(fft));
    features[ENERGY] = check_nan(energy(signal));
    features[SPECTRAL_SMOOTHNESS] = check_nan(spectral_smoothness(fft));
    features[SPECTRAL_SPREAD] = check_nan(spectral_spread(features[SPECTRAL_CENTROID], fft));
    features[SPECTRAL_DISSYMMETRY] = check_nan(spectral_dissymmetry(features[SPECTRAL_CENTROID], fft));
    
    return new FeatureSet(&features);
}
inline float check_nan(float n)
{
    return (std::fpclassify(n) == FP_NORMAL ? n : 0);
}

float FeatureExtractor::zero_crossing_rate(float *signal)
{
    unsigned int total = 0;
    for (unsigned int i = WINDOW_SIZE - 1; i--;) {
#ifdef ZCR_SCHWARZ
        total += (signal[i] < 0 ? 1 : 0) ^ (signal[i + 1] < 0 ? 1 : 0);
#else
        total += ((signal[i] * signal[i + 1]) < 0);
#endif
        __builtin_prefetch(&signal[i - 1], 0, 1);
    }
    return total *(1.0f / WINDOW_SIZE);
}

float FeatureExtractor::first_order_autocorrelation(float *signal)
{
    float total = 0;
    for (int i = 0; i < WINDOW_SIZE - 1; i++) {
        total += signal[i] * signal[i + 1];
    }
    return total *(1.0f / WINDOW_SIZE);
}

float FeatureExtractor::linear_regression(float *fft)
{
    float n = BINS - 1; // number of data points
    float Sxy = 0, Sy = 0;
    unsigned int freq = 0, Sx = 5625088, Sxx = 876286720;
    for (unsigned int i = 1; i < BINS; i++) {
        freq = i * 43;
        // Sx += freq;
        // Sxx += freq * freq;
        Sy += fft[i];
        Sxy += freq * fft[i];
    }
    //printf("Sx: %i, Sxx: %i\n", Sx, Sxx);
    return ((n * Sxy) - (Sx * Sy)) / ((n * Sxx) - pow(Sx, 2)) ;
}

float FeatureExtractor::spectral_centroid(float *fft)
{
    // skip bin 0
    // bin_number * bin_amp / amps
    float total_amp = 0;
    float bins = 0;
    for (int i = 1; i < BINS; i++) {
        bins += i * 43 * fft[i];
        total_amp += fft[i];
    }
    return bins / total_amp;
}

float FeatureExtractor::energy(float *signal)
{
    float Sxi = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        Sxi += powf(signal[i], 2.0f);
    }
    return (1.0f / WINDOW_SIZE) * Sxi;
}

float FeatureExtractor::spectral_smoothness(float *fft)
{
    float ss = 0;
    for (unsigned int i = BINS - 1; i-- != 2;) {
        ss += (20 * logf(fft[i])) - ((
                                         (20 * logf(fft[i - 1])) +
                                         (20 * logf(fft[i])) +
                                         (20 * logf(fft[i + 1]))
                                     ) / 3);
        __builtin_prefetch(&fft[i - 2], 0, 1);
    }
    return fabsf(ss);
}

float FeatureExtractor::spectral_spread(float spectral_centroid, float *fft)
{
    float top = 0, bottom = 0;
    for (int i = 1; i < BINS; i++) {
        top += fft[i] * powf((((float)i / BINS) * 22100) - spectral_centroid, 2);
        bottom += fft[i];
    }
    return sqrtf(top / bottom);
}

float FeatureExtractor::spectral_dissymmetry(float spectral_centroid, float *fft)
{
    float top = 0, bottom = 0;
    for (int i = 1; i < BINS; i++) {
        top += fft[i] * powf((((float)i / BINS) * 22100) - spectral_centroid, 3);
        bottom += fft[i];
    }
    return cbrtf(top / bottom);
}
