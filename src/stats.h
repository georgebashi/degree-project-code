#ifndef STATS_H
#define STATS_H

typedef struct Moments
{
    float mean;
    float variance;
    float skewness;
    float kurtosis;
}
Moments;

typedef struct PerceptualBlockListItem
{
    Moments *block;
    struct PerceptualBlockListItem *prev;
}
PerceptualBlockListItem;

Moments * create_perceptual_block(float **features);
Moments create_moments(float *feature_data);

#endif
