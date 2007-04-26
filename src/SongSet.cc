// $Id$

/*
SongSet class
Handles reading of all the tracks in the media library and normalising features.
*/

#include <string>
#include <vector>
#include <ftw.h>
#include <cmath>
#include <map>
#include <cfloat>

#include "common.hh"
#include "Song.hh"
#include "SongSet.hh"

SongSet* initialising;

// load songs from the specified directory
SongSet::SongSet(std::string dir)
{
    initialising = this;
    this->dir = dir;
    // walk the given dir, loading files
    ftw(dir.c_str(), read_song_files, 10);
}

// find song with the specified filename
Song* SongSet::get_by_filename(std::string filename)
{
    for (unsigned int i = this->size(); i--;) {
        if (this->at(i)->filename == filename) {
            return this->at(i);
        }
    }
    return NULL;
}

// called by the SongSet constructor, creates Song objects and adds to the vector
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

// Once all files are loaded, this function is called to normalise all features into the range 0--1
void SongSet::normalise()
{
#define MIN 0
#define MAX 1
    float song_vector[NUMBER_OF_FEATURES][NUMBER_OF_AGGREGATE_STATS][2];
    float block_vector[NUMBER_OF_FEATURES][NUMBER_OF_AGGREGATE_STATS][2];
    if (!this->size()) {
        std::cerr << "No songs loaded" << std::endl;
        exit(0);
    }
    // initialise with first values from first song
    for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
            song_vector[feature][stat][MIN] = song_vector[feature][stat][MAX] = this->at(0)->song_features->features[feature][stat];
            block_vector[feature][stat][MIN] = block_vector[feature][stat][MAX] = this->at(0)->feature_blocks->at(0)->features[feature][stat];
        }
    }
    
    // find max and min
    for (unsigned int i = 0; i < this->size(); i++) {
        Song* song = this->at(i);
        
        for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
            for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
                if (song->song_features->features[feature][stat] < song_vector[feature][stat][MIN]) {
                    song_vector[feature][stat][MIN] = song->song_features->features[feature][stat];
                }
                
                if (song->song_features->features[feature][stat] > song_vector[feature][stat][MAX]) {
                    song_vector[feature][stat][MAX] = song->song_features->features[feature][stat];
                }
                
                for (unsigned int block = 0; block < song->feature_blocks->size(); block++) {
                    if (song->feature_blocks->at(block)->features[feature][stat] < block_vector[feature][stat][MIN]) {
                        block_vector[feature][stat][MIN] = song->feature_blocks->at(block)->features[feature][stat];
                    }
                    
                    if (song->feature_blocks->at(block)->features[feature][stat] > block_vector[feature][stat][MAX]) {
                        block_vector[feature][stat][MAX] = song->feature_blocks->at(block)->features[feature][stat];
                    }
                }
            }
        }
    }
    
    // normalise!
    for (unsigned int i = 0; i < this->size(); i++) {
        Song* song = this->at(i);
        
        for (unsigned int block = 0; block < song->feature_blocks->size(); block++) {
            for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
                for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
                    song->feature_blocks->at(block)->features[feature][stat] =
                        (song->feature_blocks->at(block)->features[feature][stat] - block_vector[feature][stat][MIN]) /
                        (block_vector[feature][stat][MAX] - block_vector[feature][stat][MIN]);
                }
            }
        }
        
        for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
            for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
                song->song_features->features[feature][stat] =
                    (song->song_features->features[feature][stat] - song_vector[feature][stat][MIN]) /
                    (song_vector[feature][stat][MAX] - song_vector[feature][stat][MIN]);
            }
        }
    }
}

