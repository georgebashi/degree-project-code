#include <string>

#include <vector>
#include <ftw.h>

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
    } else
    {
        //std::cout << "!";
        std::cout << "Skipped: " << next->filename << " (" << next->feature_blocks->size() << " blocks)" << std::endl;
    }
    
    return 0;
}

void SongSet::normalise()
{
    float vector[32];
    
    // initialise with first values from first song
    for (int i = 0; i < NUMBER_OF_FEATURES; i++) {
        vector[(i * NUMBER_OF_AGGREGATE_STATS) + MEAN] = this->at(0)->feature_blocks->at(0)->mean[i];
        vector[(i * NUMBER_OF_AGGREGATE_STATS) + VARIANCE] = this->at(0)->feature_blocks->at(0)->variance[i];
        vector[(i * NUMBER_OF_AGGREGATE_STATS) + SKEWNESS] = this->at(0)->feature_blocks->at(0)->skewness[i];
        vector[(i * NUMBER_OF_AGGREGATE_STATS) + KURTOSIS] = this->at(0)->feature_blocks->at(0)->kurtosis[i];
    }
    
    // grab the numbers
    for (unsigned int i = 0; i < this->size(); i++) {
        Song* song = this->at(i);
        
        for (unsigned int block = 0; block < song->feature_blocks->size(); block++) {
            for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
            
                if (song->feature_blocks->at(block)->mean[feature] > vector[feature * MEAN]) {
                    //std::cout << "mean increased" << std::endl;
                    vector[feature + (MEAN * NUMBER_OF_FEATURES)] = song->feature_blocks->at(block)->mean[feature];
                }
                
                if (song->feature_blocks->at(block)->variance[feature] > vector[feature * VARIANCE]) {
                    //std::cout << "variance increased" << std::endl;
                    vector[feature + (VARIANCE * NUMBER_OF_FEATURES)] = song->feature_blocks->at(block)->variance[feature];
                }
                
                if (song->feature_blocks->at(block)->skewness[feature] > vector[feature * SKEWNESS]) {
                    //std::cout << "skewness increased" << std::endl;
                    vector[feature + (SKEWNESS * NUMBER_OF_FEATURES)] = song->feature_blocks->at(block)->skewness[feature];
                }
                
                if (song->feature_blocks->at(block)->kurtosis[feature] > vector[feature * KURTOSIS]) {
                    //std::cout << "kurtosis increased" << std::endl;
                    vector[feature + (KURTOSIS * NUMBER_OF_FEATURES)] = song->feature_blocks->at(block)->kurtosis[feature];
                }
            }
        }
    }
    
    // apply!
    for (unsigned int i = 0; i < this->size(); i++) {
        Song* song = this->at(i);
        
        for (unsigned int block = 0; block < song->feature_blocks->size(); block++) {
            for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
                song->feature_blocks->at(block)->mean[feature] /= vector[feature + (MEAN * NUMBER_OF_FEATURES)];
                song->feature_blocks->at(block)->variance[feature] /= vector[feature + (VARIANCE * NUMBER_OF_FEATURES)];
                song->feature_blocks->at(block)->skewness[feature] /= vector[feature + (SKEWNESS * NUMBER_OF_FEATURES)];
                song->feature_blocks->at(block)->kurtosis[feature] /= vector[feature + (KURTOSIS * NUMBER_OF_FEATURES)];
            }
        }
        
        for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
            song->song_features->mean[feature] /= vector[feature + (MEAN * NUMBER_OF_FEATURES)];
            song->song_features->variance[feature] /= vector[feature + (VARIANCE * NUMBER_OF_FEATURES)];
            song->song_features->skewness[feature] /= vector[feature + (SKEWNESS * NUMBER_OF_FEATURES)];
            song->song_features->kurtosis[feature] /= vector[feature + (KURTOSIS * NUMBER_OF_FEATURES)];
        }
    }
}
