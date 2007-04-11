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
        mean[feature] = get_mean(data, data_points);
        variance[feature] = get_variance(data, mean[feature], data_points);
        float stdev = get_stdev(variance[feature]);
        skewness[feature] = get_skewness(data, mean[feature], stdev, data_points);
        kurtosis[feature] = get_kurtosis(data, mean[feature], stdev, data_points);
    }
}

FeatureGroup::FeatureGroup(FeatureGroup* fg1, FeatureGroup* fg2, float dist)
{
    for (int i = 0; i < NUMBER_OF_FEATURES; i++) {
        this->mean[i] = fg1->mean[i] + (dist * (fg2->mean[i] - fg1->mean[i]));
        this->variance[i] = fg1->variance[i] + (dist * (fg2->variance[i] - fg1->variance[i]));
        this->skewness[i] = fg1->skewness[i] + (dist * (fg2->skewness[i] - fg1->skewness[i]));
        this->kurtosis[i] = fg1->kurtosis[i] + (dist * (fg2->kurtosis[i] - fg1->kurtosis[i]));
    }
}

float FeatureGroup::compare(FeatureGroup* other, float* weights)
{
    float mean_diff = 0, variance_diff = 0, skewness_diff = 0, kurtosis_diff = 0;
    for (int i = NUMBER_OF_FEATURES; i--;) {
        mean_diff += fabsf(fabsf(mean[i]) - fabsf(other->mean[i])) * WEIGHT(i, MEAN);
        CHECK(mean_diff)
        variance_diff += fabsf(fabsf(variance[i]) - fabsf(other->variance[i])) * WEIGHT(i, VARIANCE);
        CHECK(variance_diff)
        skewness_diff += fabsf(fabsf(skewness[i]) - fabsf(other->skewness[i])) * WEIGHT(i, SKEWNESS);
        CHECK(skewness_diff)
        kurtosis_diff += fabsf(fabsf(kurtosis[i]) - fabsf(other->kurtosis[i])) * WEIGHT(i, KURTOSIS);
        CHECK(kurtosis_diff)
    }
    return mean_diff + variance_diff + skewness_diff + kurtosis_diff;
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
        total += pow(data[i] - mean, 2);
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
        total += pow(data[i] - mean, 3);
    }
    return total / (n * pow(stdev, 3));
}
float get_kurtosis(float *data, float mean, float stdev, int n)
{
    float total = 0;
    for (int i = 0; i < n; i++) {
        total += pow(data[i] - mean, 4);
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
    features[SPECTRAL_INTENSITY] = check_nan(spectral_intensity(fft));
    features[SPECTRAL_SMOOTHNESS] = check_nan(spectral_smoothness(fft));
    features[SPECTRAL_SPREAD] = check_nan(spectral_spread(features[SPECTRAL_CENTROID], fft));
    features[SPECTRAL_DISSYMMETRY] = check_nan(spectral_dissymmetry(features[SPECTRAL_CENTROID], fft));
    
    return new FeatureSet(&features);
}
inline float check_nan(float n)
{
    return (std::isnormal(n) ? n : 0);
}

float FeatureExtractor::zero_crossing_rate(float *signal)
{
    unsigned int total = 0;
    for (unsigned int i = WINDOW_SIZE - 1; i--;) {
#ifdef ZCR_SCHWARZ
        total += (signal[i] < 0 ? 1 : 0) ^(signal[i + 1] < 0 ? 1 : 0);
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
    /*
     sum up
      x * y
      x
      y
      pow(x, 2)
    */
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

float FeatureExtractor::spectral_intensity(float *fft)
{
    return fft[0];
}

float FeatureExtractor::spectral_smoothness(float *fft)
{
    float ss = 0;
    //for (unsigned int i = 2; i < BINS - 1; i++) {
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
        top += fft[i] * pow((((float)i / BINS) * 22100) - spectral_centroid, 2);
        bottom += fft[i];
    }
    return sqrtf(top / bottom);
}

float FeatureExtractor::spectral_dissymmetry(float spectral_centroid, float *fft)
{
    float top = 0, bottom = 0;
    for (int i = 1; i < BINS; i++) {
        top += fft[i] * pow((((float)i / BINS) * 22100) - spectral_centroid, 3);
        bottom += fft[i];
    }
    return cbrtf(top / bottom);
}
