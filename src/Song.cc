// $Id$

/*
Song class
Handles loading and saving of features to disk in .vec files
*/

#include <vector>
#include <string>
#include <fstream>
#include <cmath>

#include "common.hh"
#include "Features.hh"
#include "Song.hh"

float (*comparison_weights)[NUMBER_OF_AGGREGATE_STATS];

// create a new song with the specified filename
Song::Song(std::string filename)
{
    this->filename = filename;
    feature_blocks = new std::vector<FeatureGroup*>();
}

// destroy the song and free memory
Song::~Song()
{
    for (unsigned int i = 0; i < feature_blocks->size(); i++) {
        delete feature_blocks->at(i);
    }
    feature_blocks->clear();
    delete feature_blocks;
    delete song_features;
}

// load a song file with the specified path and filename
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

// helper function to read a feature group from disk
FeatureGroup* readFeatureGroup(std::ifstream* input)
{
    FeatureGroup* new_group = new FeatureGroup();
    for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
            input->read((char *)&new_group->features[feature][stat], sizeof(float));
        }
    }
    return new_group;
}

// add a feature block to a new song being processed
void Song::addBlock(FeatureGroup* block)
{
    feature_blocks->push_back(block);
}

// set the song features block for a new song
void Song::setSongFeatures(FeatureGroup* song_features)
{
    this->song_features = song_features;
}

// write the song file to disk
void Song::write(std::ofstream* output)
{
    writeFeatureGroup(output, song_features);
    
    int num_blocks = feature_blocks->size();
    output->write((char *)&num_blocks, sizeof(int));
    for (unsigned int i = 0; i < feature_blocks->size(); i++) {
        writeFeatureGroup(output, feature_blocks->at(i));
    }
}

// helper function to write a feature group to disk
void writeFeatureGroup(std::ofstream* output, FeatureGroup* feature_group)
{
    for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
            output->write((char *)&feature_group->features[feature][stat], sizeof(float));
        }
    }
}

// two helper functions to check for invalid numbers being stored to disk
bool float_bad(float f)
{
    return !std::isnormal(f);
}

bool Song::has_nan()
{
    for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
            if (float_bad(song_features->features[feature][stat])) {
                return 1;
            }
        }
    }
    
    for (unsigned int block = 0; block < feature_blocks->size(); block++) {
        for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
            for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
                if (float_bad(feature_blocks->at(block)->features[feature][stat])) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// get the artist of this song by parsing the file path
std::string Song::get_artist()
{
    unsigned int slash_pos[filename.length()];
    slash_pos[0] = filename.length();
    for (unsigned int i = 1; i < filename.length(); i++) {
        slash_pos[i] = filename.rfind("/", slash_pos[i - 1] - 1);
        if (slash_pos[i] == std::string::npos) { break; }
    }
    // between 3 and 2
    return filename.substr(slash_pos[3] + 1, slash_pos[2] - slash_pos[3] - 1);
}

// get the album that this song is on by parsing the file path
std::string Song::get_album()
{
    unsigned int slash_pos[filename.length()];
    slash_pos[0] = filename.length();
    for (unsigned int i = 1; i < filename.length(); i++) {
        slash_pos[i] = filename.rfind("/", slash_pos[i - 1] - 1);
        if (slash_pos[i] == std::string::npos) { break; }
    }
    // between 3 and 2
    return filename.substr(slash_pos[2] + 1, slash_pos[1] - slash_pos[2] - 1);
}

// deprecated function to compare two songs without using weights and the default comparison method
float Song::compare(Song* other)
{
    float weights[NUMBER_OF_FEATURES][NUMBER_OF_AGGREGATE_STATS] =
        {
            { 1, 1, 1, 1 },
            { 1, 1, 1, 1 },
            { 1, 1, 1, 1 },
            { 1, 1, 1, 1 },
            { 1, 1, 1, 1 },
            { 1, 1, 1, 1 },
            { 1, 1, 1, 1 },
            { 1, 1, 1, 1 }
        };
    return compare(other, weights, ORDERED);
}

// similarity metric code, compare two songs and return a distance
float Song::compare(Song* other, float (*weights)[NUMBER_OF_AGGREGATE_STATS], int comparison_function)
{
    float song_diff, block_diff;
    block_diff = 0;
    song_diff = song_features->compare(other->song_features, weights);
    // choose the correct algorithm and calcuate distance accordingly
    switch (comparison_function) {
        case ORDERED: {
            int smallest_feature_block_count = feature_blocks->size() < other->feature_blocks->size() ? feature_blocks->size() : other->feature_blocks->size();
            for (int i = 0; i < smallest_feature_block_count; i++) {
                block_diff += feature_blocks->at(i)->compare(other->feature_blocks->at(i), weights);
            }
            return (song_diff * SONG_FEATURE_WEIGHT) + ((block_diff / smallest_feature_block_count) * BLOCK_FEATURE_WEIGHT);
        }
        case ORDERED_AREA: {
            int smallest_feature_block_count = feature_blocks->size() < other->feature_blocks->size() ? feature_blocks->size() : other->feature_blocks->size();
            for (int i = 0; i < smallest_feature_block_count; i++) {
                if (i != 0) {
                    block_diff += feature_blocks->at(i)->compare(other->feature_blocks->at(i - 1), weights);
                }
                block_diff += feature_blocks->at(i)->compare(other->feature_blocks->at(i), weights);
                if (i != smallest_feature_block_count - 1) {
                    block_diff += feature_blocks->at(i)->compare(other->feature_blocks->at(i + 1), weights);
                }
            }
            return (song_diff * SONG_FEATURE_WEIGHT) + ((block_diff / smallest_feature_block_count) * BLOCK_FEATURE_WEIGHT);
        }
        case SORTED: {
            comparison_weights = weights;
            std::sort(feature_blocks->begin(), feature_blocks->end(), fg_cmp);
            std::sort(other->feature_blocks->begin(), other->feature_blocks->end(), fg_cmp);
            int smallest_feature_block_count = feature_blocks->size() < other->feature_blocks->size() ? feature_blocks->size() : other->feature_blocks->size();
            for (int i = 0; i < smallest_feature_block_count; i++) {
                block_diff += feature_blocks->at(i)->compare(other->feature_blocks->at(i), weights);
            }
            return (song_diff * SONG_FEATURE_WEIGHT) + ((block_diff / smallest_feature_block_count) * BLOCK_FEATURE_WEIGHT);
            break;
        }
        case TOTAL: {
            unsigned int i = 0, j = 0, i_size = feature_blocks->size(), j_size = other->feature_blocks->size();
            for (i = 0; i < i_size; i++) {
                for (j = 0; j < j_size; j++) {
                    block_diff += feature_blocks->at(i)->compare(other->feature_blocks->at(j), weights);
                }
            }
            return (song_diff * SONG_FEATURE_WEIGHT) + ((block_diff / (i * j)) * BLOCK_FEATURE_WEIGHT);
        }
    }
    return 0;
}

// comparator used to sort feature blocks in the sorted comparison
bool fg_cmp(FeatureGroup* fg1, FeatureGroup* fg2)
{
    float first_total = 0, second_total = 0;
    float (*weights)[NUMBER_OF_AGGREGATE_STATS] = comparison_weights;
    for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
            first_total += fg1->features[feature][stat] * weights[feature][stat];
            second_total += fg2->features[feature][stat] * weights[feature][stat];
        }
    }
    
    return first_total > second_total;
}

