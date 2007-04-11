// $Id$

#include <string>
#include <vector>
#include <ftw.h>
#include <cmath>

#include "common.hh"
#include "Song.hh"
#include "SongSet.hh"

SongSet* initialising;

SongSet::SongSet(std::string dir)
{
    initialising = this;
    this->dir = dir;
    // walk the given dir, loading files
    ftw(dir.c_str(), read_song_files, 10);
}

Song* SongSet::get_by_filename(std::string filename)
{
    for (unsigned int i = this->size(); i--;) {
        if (this->at(i)->filename == filename) {
            return this->at(i);
        }
    }
    return NULL;
}

int read_song_files(const char *fpath, const struct stat *sb, int typeflag)
{
    std::string filename(fpath);
    if (typeflag != FTW_F || filename.length() < 4 || filename.compare(filename.length() - 4, 4, ".vec") != 0)
    {
        return 0;
    }
    
    Song* next = new Song(initialising->dir, fpath);
    if (!next->has_nan())
    {
        initialising->push_back(next);
    }
    
    return 0;
}

void SongSet::normalise()
{
    float song_vector[32];
    float block_vector[32];
    
    // initialise with first values from first song
    for (int i = 0; i < NUMBER_OF_FEATURES; i++) {
        song_vector[(i * NUMBER_OF_AGGREGATE_STATS) + MEAN] = this->at(0)->song_features->mean[i];
        song_vector[(i * NUMBER_OF_AGGREGATE_STATS) + VARIANCE] = this->at(0)->song_features->variance[i];
        song_vector[(i * NUMBER_OF_AGGREGATE_STATS) + SKEWNESS] = this->at(0)->song_features->skewness[i];
        song_vector[(i * NUMBER_OF_AGGREGATE_STATS) + KURTOSIS] = this->at(0)->song_features->kurtosis[i];
        
        block_vector[(i * NUMBER_OF_AGGREGATE_STATS) + MEAN] = this->at(0)->feature_blocks->at(0)->mean[i];
        block_vector[(i * NUMBER_OF_AGGREGATE_STATS) + VARIANCE] = this->at(0)->feature_blocks->at(0)->variance[i];
        block_vector[(i * NUMBER_OF_AGGREGATE_STATS) + SKEWNESS] = this->at(0)->feature_blocks->at(0)->skewness[i];
        block_vector[(i * NUMBER_OF_AGGREGATE_STATS) + KURTOSIS] = this->at(0)->feature_blocks->at(0)->kurtosis[i];
        
        
    }
    
    // grab the numbers
    for (unsigned int i = 0; i < this->size(); i++) {
        Song* song = this->at(i);
        
        for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
            if (fabsf(song->song_features->mean[feature]) > song_vector[feature + (MEAN * NUMBER_OF_FEATURES)]) {
                song_vector[feature + (MEAN * NUMBER_OF_FEATURES)] = fabsf(song->song_features->mean[feature]);
            }
            
            if (fabsf(song->song_features->variance[feature]) > song_vector[feature + (VARIANCE * NUMBER_OF_FEATURES)]) {
                song_vector[feature + (VARIANCE * NUMBER_OF_FEATURES)] = fabsf(song->song_features->variance[feature]);
            }
            
            if (fabsf(song->song_features->skewness[feature]) > song_vector[feature + (SKEWNESS * NUMBER_OF_FEATURES)]) {
                song_vector[feature + (SKEWNESS * NUMBER_OF_FEATURES)] = fabsf(song->song_features->skewness[feature]);
            }
            
            if (fabsf(song->song_features->kurtosis[feature]) > song_vector[feature + (KURTOSIS * NUMBER_OF_FEATURES)]) {
                song_vector[feature + (KURTOSIS * NUMBER_OF_FEATURES)] = fabsf(song->song_features->kurtosis[feature]);
            }
            
            
            for (unsigned int block = 0; block < song->feature_blocks->size(); block++) {
                if (fabsf(song->feature_blocks->at(block)->mean[feature]) > block_vector[feature + (MEAN * NUMBER_OF_FEATURES)]) {
                    block_vector[feature + (MEAN * NUMBER_OF_FEATURES)] = fabsf(song->feature_blocks->at(block)->mean[feature]);
                }
                
                if (fabsf(song->feature_blocks->at(block)->variance[feature]) > block_vector[feature + (VARIANCE * NUMBER_OF_FEATURES)]) {
                    block_vector[feature + (VARIANCE * NUMBER_OF_FEATURES)] = fabsf(song->feature_blocks->at(block)->variance[feature]);
                }
                
                if (fabsf(song->feature_blocks->at(block)->skewness[feature]) > block_vector[feature + (SKEWNESS * NUMBER_OF_FEATURES)]) {
                    block_vector[feature + (SKEWNESS * NUMBER_OF_FEATURES)] = fabsf(song->feature_blocks->at(block)->skewness[feature]);
                }
                
                if (fabsf(song->feature_blocks->at(block)->kurtosis[feature]) > block_vector[feature + (KURTOSIS * NUMBER_OF_FEATURES)]) {
                    block_vector[feature + (KURTOSIS * NUMBER_OF_FEATURES)] = fabsf(song->feature_blocks->at(block)->kurtosis[feature]);
                }
            }
        }
    }
    
    // apply!
    for (unsigned int i = 0; i < this->size(); i++) {
        Song* song = this->at(i);
        
        for (unsigned int block = 0; block < song->feature_blocks->size(); block++) {
            for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
                song->feature_blocks->at(block)->mean[feature] /= block_vector[feature + (MEAN * NUMBER_OF_FEATURES)];
                song->feature_blocks->at(block)->variance[feature] /= block_vector[feature + (VARIANCE * NUMBER_OF_FEATURES)];
                song->feature_blocks->at(block)->skewness[feature] /= block_vector[feature + (SKEWNESS * NUMBER_OF_FEATURES)];
                song->feature_blocks->at(block)->kurtosis[feature] /= block_vector[feature + (KURTOSIS * NUMBER_OF_FEATURES)];
            }
        }
        
        for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
            song->song_features->mean[feature] /= song_vector[feature + (MEAN * NUMBER_OF_FEATURES)];
            song->song_features->variance[feature] /= song_vector[feature + (VARIANCE * NUMBER_OF_FEATURES)];
            song->song_features->skewness[feature] /= song_vector[feature + (SKEWNESS * NUMBER_OF_FEATURES)];
            song->song_features->kurtosis[feature] /= song_vector[feature + (KURTOSIS * NUMBER_OF_FEATURES)];
        }
    }
}
