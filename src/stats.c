#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "analysis.h"
#include "stats.h"

Moments * create_perceptual_block(float **features)
{
    float *feature_data = malloc(sizeof(float) * WINDOWS_PER_BLOCK);
    int feature, i;
    Moments * block = malloc(sizeof(Moments) * ANALYSIS_OUTPUTS);
    
    for (feature = 0; feature < ANALYSIS_OUTPUTS; feature++) {
        for (i = 0; i < WINDOWS_PER_BLOCK; i++) {
            feature_data[i] = features[i][feature];
        }
        block[feature] = create_moments(feature_data);
    }
    
    free(feature_data);
    return block;
}

Moments create_moments(float *feature_data)
{
    Moments output;
    int i = 0, total = 0;
    float mean = 0, stdev = 0;
    
    // mean
    for (i = 0; i < WINDOWS_PER_BLOCK; i++) {
        total += feature_data[i];
    }
    mean = total / WINDOWS_PER_BLOCK;
    output.mean = mean;
    total = 0;
    
    // variance
    for (i = 0; i < WINDOWS_PER_BLOCK; i++) {
        total += pow(feature_data[i] - mean, 2);
    }
    output.variance = total / (WINDOWS_PER_BLOCK - 1);
    stdev = sqrt(output.variance); /****/
    total = 0;
    
    // skewness
    for (i = 0; i < WINDOWS_PER_BLOCK; i++) {
        total += pow(feature_data[i] - mean, 3); /****/
    }
    output.skewness = total / (WINDOWS_PER_BLOCK * pow(stdev, 3)); /****/
    total = 0;
    
    // kurtosis
    for (i = 0; i < WINDOWS_PER_BLOCK; i++) {
        total += pow(feature_data[i] - mean, 4); /****/
    }
    output.skewness = total / (WINDOWS_PER_BLOCK * pow(stdev, 4)); /****/
    total = 0;
    
    return output;
}
