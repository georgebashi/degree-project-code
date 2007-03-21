#include <cmath>
#include <vector>
#include <assert.h>
#include <cfloat>

#include "common.hh"
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
        float data[WINDOWS_PER_BLOCK];
        
        for (unsigned int i = 0; i < WINDOWS_PER_BLOCK; i++) {
            data[i] = sets->at(i)->features[feature];
        }
        mean[feature] = get_mean(data, WINDOWS_PER_BLOCK);
        variance[feature] = get_variance(data, mean[feature], WINDOWS_PER_BLOCK);
        float stdev = get_stdev(variance[feature]);
        skewness[feature] = get_skewness(data, mean[feature], stdev, WINDOWS_PER_BLOCK);
        kurtosis[feature] = get_kurtosis(data, mean[feature], stdev, WINDOWS_PER_BLOCK);
    }
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
    
    features[ZERO_CROSSING_RATE] = zero_crossing_rate(signal);
    features[FIRST_ORDER_AUTOCORRELATION] = first_order_autocorrelation(signal);
    features[LINEAR_REGRESSION] = linear_regression(fft);
    features[SPECTRAL_CENTROID] = spectral_centroid(fft);
    features[SPECTRAL_INTENSITY] = spectral_intensity(fft);
    features[SPECTRAL_SMOOTHNESS] = spectral_smoothness(fft);
    features[SPECTRAL_SPREAD] = spectral_spread(features[SPECTRAL_CENTROID], fft);
    features[SPECTRAL_DISSYMMETRY] = spectral_dissymmetry(features[SPECTRAL_CENTROID], fft);
    
    return new FeatureSet(&features);
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
    float sx = 5625088.0f, sxx = 82480665344.0f, sy = 0, syy = 0, sxy = 0;
    for (int i = 1; i < BINS; i++) {
        sy += fft[i];
        syy += pow(fft[i], 2);
        sxy += i * 43 * fft[i];
    }
    return ((BINS * sxy) - (sx * sy)) / ((BINS * sxx) - pow(sx, 2));
}

float FeatureExtractor::spectral_centroid(float *fft)
{
    // skip bin 0
    // bin_number * bin_amp / amps
    float total_amp = 0;
    float bins = 0;
    for (int i = 1; i < BINS; i++) {
        bins += i * fft[i];
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



