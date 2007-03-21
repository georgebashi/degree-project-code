#include <string.h>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <assert.h>
#include <cfloat>

#include "common.hh"
#include "Features.hh"

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
    std::string filepath;
    if (path.compare(path.length() - 1, 1, "/") != 0) {
        filepath = path + "/" + filename;
    } else {
        filepath = path + filename;
    }
    
    std::ifstream input(filepath.c_str(), std::ios::binary);
    
    // read the file name
    int filename_length;
    input.read((char *) &filename_length, sizeof(int));
    
    char filename_cstr[filename_length];
    input.read(filename_cstr, sizeof(char) * filename_length);
    this->filename = std::string(filename_cstr);
    
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
    int filename_length = filename.length() + 1;
    output->write((char *)&filename_length, sizeof(int));
    output->write(filename.c_str(), filename.length() + 1);
    
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


float Song::compare(Song* other)
{
    // SIMILARITY MEASURE!
    float total_diff = compareFeatureGroup(song_features, other->song_features);
    int smallest_feature_block_count = feature_blocks->size() < other->feature_blocks->size() ? feature_blocks->size() : other->feature_blocks->size();
    for (int i = 0; i < smallest_feature_block_count; i++) {
        total_diff += compareFeatureGroup(feature_blocks->at(i), other->feature_blocks->at(i));
    }
    return total_diff;
}

float compareFeatureGroup(FeatureGroup* a, FeatureGroup* b)
{
    float mean_diff = 0, variance_diff = 0, skewness_diff = 0, kurtosis_diff = 0;
    for (int i = NUMBER_OF_FEATURES; i--;) {
        mean_diff += fabsf(a->mean[i]) - fabsf(b->mean[i]);
        variance_diff += fabsf(a->variance[i]) - fabsf(b->variance[i]);
        skewness_diff += fabsf(a->skewness[i]) - fabsf(b->skewness[i]);
        kurtosis_diff += fabsf(a->kurtosis[i]) - fabsf(b->kurtosis[i]);
    }
    return mean_diff + variance_diff + skewness_diff + kurtosis_diff;
}

