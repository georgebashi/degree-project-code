// $Id$

#include <vector>
#include <string>
#include <fstream>
#include <cmath>

#include "common.hh"
#include "Song.hh"


Song::Song(std::string filename)
{
    this->filename = filename;
    feature_blocks = new std::vector<FeatureGroup*>();
}

Song::~Song()
{
    for (unsigned int i = 0; i < feature_blocks->size(); i++) {
        delete feature_blocks->at(i);
    }
    feature_blocks->clear();
    delete feature_blocks;
    delete song_features;
}

Song::Song(std::string path, std::string filename)
{
    this->filename = filename;
    
    std::string filepath;
    if (path.compare(path.length() - 1, 1, "/") != 0) {
        filepath = path + "/" + filename;
    } else {
        filepath = path + filename;
    }
    std::ifstream input(filepath.c_str(), std::ios::binary);
    
    // read the song features
    this->song_features = readFeatureGroup(&input);
    
    // read the feature blocks
    int num_blocks;
    input.read((char *)&num_blocks, sizeof(int));
    this->feature_blocks = new std::vector<FeatureGroup *>();
    for (int i = 0; i < num_blocks; i++) {
        this->feature_blocks->push_back(readFeatureGroup(&input));
    }
}

FeatureGroup* readFeatureGroup(std::ifstream* input)
{
    FeatureGroup* new_group = new FeatureGroup();
    for (int i = 0; i < NUMBER_OF_FEATURES; i++) {
        input->read((char *)&new_group->mean[i], sizeof(float));
        input->read((char *)&new_group->variance[i], sizeof(float));
        input->read((char *)&new_group->skewness[i], sizeof(float));
        input->read((char *)&new_group->kurtosis[i], sizeof(float));
    }
    return new_group;
}

void Song::addBlock(FeatureGroup* block)
{
    feature_blocks->push_back(block);
}

void Song::setSongFeatures(FeatureGroup* song_features)
{
    this->song_features = song_features;
}

void Song::write(std::ofstream* output)
{
    writeFeatureGroup(output, song_features);
    
    int num_blocks = feature_blocks->size();
    output->write((char *)&num_blocks, sizeof(int)); //
    for (unsigned int i = 0; i < feature_blocks->size(); i++) {
        writeFeatureGroup(output, feature_blocks->at(i));
    }
}

void writeFeatureGroup(std::ofstream* output, FeatureGroup* feature_group)
{
    for (int i = 0; i < NUMBER_OF_FEATURES; i++) {
        output->write((char *)&feature_group->mean[i], sizeof(float));
        output->write((char *)&feature_group->variance[i], sizeof(float));
        output->write((char *)&feature_group->skewness[i], sizeof(float));
        output->write((char *)&feature_group->kurtosis[i], sizeof(float));
    }
}

bool float_bad(float f)
{
    return !std::isnormal(f);
}

bool Song::has_nan()
{
    for (int i = 0; i < NUMBER_OF_FEATURES; i++) {
        if (float_bad(song_features->mean[i]) || float_bad(song_features->variance[i]) || float_bad(song_features->skewness[i]) || float_bad(song_features->kurtosis[i])) {
            return 1;
        }
    }
    
    for (unsigned int i = 0; i < feature_blocks->size(); i++) {
        for (int j = 0; j < NUMBER_OF_FEATURES; j++) {
            if (float_bad(feature_blocks->at(i)->mean[j]) || float_bad(feature_blocks->at(i)->variance[j]) || float_bad(feature_blocks->at(i)->skewness[j]) || float_bad(feature_blocks->at(i)->kurtosis[j])) {
                return 1;
            }
        }
    }
    return 0;
}



float Song::compare(Song* other)
{
    float weights[/*NUMBER_OF_FEATURES * NUMBER_OF_AGGREGATE_STATS*/ 32] =
        {
            1, 0, 0, 0,
            1, 0, 0, 0,
            1, 0, 0, 0,
            1, 0, 0, 0,
            1, 0, 0, 0,
            1, 0, 0, 0,
            1, 0, 0, 0,
            1, 0, 0, 0
        };
    return compare(other, weights, ORDERED);
}

float* comparison_weights;

float Song::compare(Song* other, float* weights, int comparison_function)
{
    //std::cout << comparison_function;
    float song_diff, block_diff;
    block_diff = 0;
    song_diff = compareFeatureGroup(song_features, other->song_features, weights);
    CHECK(song_diff)
    
    switch (comparison_function) {
        case ORDERED: {
            int smallest_feature_block_count = feature_blocks->size() < other->feature_blocks->size() ? feature_blocks->size() : other->feature_blocks->size();
            for (int i = 0; i < smallest_feature_block_count; i++) {
                block_diff += compareFeatureGroup(feature_blocks->at(i), other->feature_blocks->at(i), weights);
                CHECK(block_diff)
            }
            CHECK((song_diff * SONG_FEATURE_WEIGHT) + ((block_diff / smallest_feature_block_count) * BLOCK_FEATURE_WEIGHT))
            return (song_diff * SONG_FEATURE_WEIGHT) + ((block_diff / smallest_feature_block_count) * BLOCK_FEATURE_WEIGHT);
        }
        case ORDERED_AREA: {
            int smallest_feature_block_count = feature_blocks->size() < other->feature_blocks->size() ? feature_blocks->size() : other->feature_blocks->size();
            for (int i = 0; i < smallest_feature_block_count; i++) {
                if (i != 0) {
                    block_diff += compareFeatureGroup(feature_blocks->at(i), other->feature_blocks->at(i - 1), weights);
                }
                block_diff += compareFeatureGroup(feature_blocks->at(i), other->feature_blocks->at(i), weights);
                if (i != smallest_feature_block_count - 1) {
                    block_diff += compareFeatureGroup(feature_blocks->at(i), other->feature_blocks->at(i + 1), weights);
                }
                CHECK(block_diff)
            }
            CHECK((song_diff * SONG_FEATURE_WEIGHT) + ((block_diff / smallest_feature_block_count) * BLOCK_FEATURE_WEIGHT))
            return (song_diff * SONG_FEATURE_WEIGHT) + ((block_diff / smallest_feature_block_count) * BLOCK_FEATURE_WEIGHT);
        }
        case SORTED: {
            comparison_weights = weights;
            std::sort(feature_blocks->begin(), feature_blocks->end(), fg_cmp);
            std::sort(other->feature_blocks->begin(), other->feature_blocks->end(), fg_cmp);
            int smallest_feature_block_count = feature_blocks->size() < other->feature_blocks->size() ? feature_blocks->size() : other->feature_blocks->size();
            for (int i = 0; i < smallest_feature_block_count; i++) {
                block_diff += compareFeatureGroup(feature_blocks->at(i), other->feature_blocks->at(i), weights);
                CHECK(block_diff)
            }
            CHECK((song_diff * SONG_FEATURE_WEIGHT) + ((block_diff / smallest_feature_block_count) * BLOCK_FEATURE_WEIGHT))
            return (song_diff * SONG_FEATURE_WEIGHT) + ((block_diff / smallest_feature_block_count) * BLOCK_FEATURE_WEIGHT);
            break;
        }
        case TOTAL: {
            unsigned int i = 0, j = 0;
            for (i = 0; i < feature_blocks->size(); i++) {
                for (j = 0; j < other->feature_blocks->size(); j++) {
                    block_diff += compareFeatureGroup(feature_blocks->at(i), other->feature_blocks->at(j), weights);
                    CHECK(block_diff)
                }
            }
            return (song_diff * SONG_FEATURE_WEIGHT) + ((block_diff / (i * j)) * BLOCK_FEATURE_WEIGHT);
        }
    }
    return 0;
}

bool fg_cmp(FeatureGroup* fg1, FeatureGroup* fg2)
{
    float first_total = 0, second_total = 0;
    float* weights = comparison_weights;
    for (int i = 0; i < NUMBER_OF_FEATURES; i++) {
        first_total += (fg1->mean[i] * WEIGHT(i, MEAN)) +
                       (fg1->variance[i] * WEIGHT(i, VARIANCE)) +
                       (fg1->skewness[i] * WEIGHT(i, SKEWNESS)) +
                       (fg1->kurtosis[i] * WEIGHT(i, KURTOSIS));
        second_total += (fg2->mean[i] * WEIGHT(i, MEAN)) +
                        (fg2->variance[i] * WEIGHT(i, VARIANCE)) +
                        (fg2->skewness[i] * WEIGHT(i, SKEWNESS)) +
                        (fg2->kurtosis[i] * WEIGHT(i, KURTOSIS));
    }
    
    return first_total > second_total;
}


float compareFeatureGroup(FeatureGroup* a, FeatureGroup* b, float* weights)
{
    float mean_diff = 0, variance_diff = 0, skewness_diff = 0, kurtosis_diff = 0;
    for (int i = NUMBER_OF_FEATURES; i--;) {
        mean_diff += fabsf(fabsf(a->mean[i]) - fabsf(b->mean[i])) * WEIGHT(i, MEAN);
        CHECK(mean_diff)
        variance_diff += fabsf(fabsf(a->variance[i]) - fabsf(b->variance[i])) * WEIGHT(i, VARIANCE);
        CHECK(variance_diff)
        skewness_diff += fabsf(fabsf(a->skewness[i]) - fabsf(b->skewness[i])) * WEIGHT(i, SKEWNESS);
        CHECK(skewness_diff)
        kurtosis_diff += fabsf(fabsf(a->kurtosis[i]) - fabsf(b->kurtosis[i])) * WEIGHT(i, KURTOSIS);
        CHECK(kurtosis_diff)
    }
    return mean_diff + variance_diff + skewness_diff + kurtosis_diff;
}
