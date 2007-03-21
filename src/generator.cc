#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <popt.h>
#include <vector>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <cfloat>

#include "common.hh"
#include "Features.hh"
#include "Song.hh"
#include "generator.hh"


int main(int argc, const char *argv[])
{
    // read options
    poptContext context = poptGetContext("generator", argc, argv, options, 0);
    poptSetOtherOptionHelp(context, "[OPTION...] file.wav");
    
    // parse
    display_version = 0;
    dir = ".";
    poptGetNextOpt(context);
    
    // need at least 1 file arg
    if (argc == 1) {
        poptPrintUsage(context, stdout, 0);
        exit(EXIT_SUCCESS);
    }
    
    if (display_version) {
        std::cout << "Playlist Generator: svn-$Revision$" << std::endl;
        exit(EXIT_SUCCESS);
    }
    
    // get file list
    const char ** key_songs = poptGetArgs(context);
    DIR* song_dir = opendir(dir);
    std::vector<Song *> song_vectors;
    dirent* song_file;
    while ((song_file = readdir(song_dir)) != NULL) {
        std::string filename(song_file->d_name);
        if (filename.length() < 4 || filename.compare(filename.length() - 4, 4, ".vec") != 0) { continue; }
        song_vectors.push_back(new Song(std::string(dir), song_file->d_name));
        std::cout << song_vectors.back()->filename << std::endl;
    }
    
    // find normalisation vector
    float* normalisation_vector = new float[NUMBER_OF_FEATURES * NUMBER_OF_AGGREGATE_STATS];
    getNormalisationVector(song_vectors, normalisation_vector);
    normalise(song_vectors, normalisation_vector);
    
    /*for (int i = 0; i < NUMBER_OF_FEATURES * NUMBER_OF_AGGREGATE_STATS; i++) {
        std::cout << normalisation_vector[i] << std::endl;
    }*/
    
    for (unsigned int block = 0; block < song_vectors.at(0)->feature_blocks->size(); block++) {
        printf("block %d:\n", block);
        for (int i = 0; i < NUMBER_OF_FEATURES; i++) {
            printf("  feature %d: %f,%f,%f,%f\n",
                   i,
                   song_vectors.at(0)->feature_blocks->at(block)->mean[i],
                   song_vectors.at(0)->feature_blocks->at(block)->variance[i],
                   song_vectors.at(0)->feature_blocks->at(block)->skewness[i],
                   song_vectors.at(0)->feature_blocks->at(block)->kurtosis[i]
                  );
        }
    }
    poptFreeContext(context);
    return EXIT_SUCCESS;
}

void getNormalisationVector(std::vector<Song *> songs, float* vector)
{
    // initialise with first values from first song
    for (int i = NUMBER_OF_FEATURES * NUMBER_OF_AGGREGATE_STATS; i--;) {
        vector[i + (MEAN * NUMBER_OF_FEATURES)] = songs.at(0)->feature_blocks->at(0)->mean[i];
        vector[i + (VARIANCE * NUMBER_OF_FEATURES)] = songs.at(0)->feature_blocks->at(0)->variance[i];
        vector[i + (SKEWNESS * NUMBER_OF_FEATURES)] = songs.at(0)->feature_blocks->at(0)->skewness[i];
        vector[i + (KURTOSIS * NUMBER_OF_FEATURES)] = songs.at(0)->feature_blocks->at(0)->kurtosis[i];
    }
    
    for (unsigned int i = 0; i < songs.size(); i++) {
        Song* song = songs.at(i);
        
        for (unsigned int block = 0; block < song->feature_blocks->size(); block++) {
            for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
            
                if (song->feature_blocks->at(block)->mean[feature] > vector[feature * MEAN]) {
                    std::cout << "mean increased" << std::endl;
                    vector[feature + (MEAN * NUMBER_OF_FEATURES)] = song->feature_blocks->at(block)->mean[feature];
                }
                
                if (song->feature_blocks->at(block)->variance[feature] > vector[feature * VARIANCE]) {
                    std::cout << "variance increased" << std::endl;
                    vector[feature + (VARIANCE * NUMBER_OF_FEATURES)] = song->feature_blocks->at(block)->variance[feature];
                }
                
                if (song->feature_blocks->at(block)->skewness[feature] > vector[feature * SKEWNESS]) {
                    std::cout << "skewness increased" << std::endl;
                    vector[feature + (SKEWNESS * NUMBER_OF_FEATURES)] = song->feature_blocks->at(block)->skewness[feature];
                }
                
                if (song->feature_blocks->at(block)->kurtosis[feature] > vector[feature * KURTOSIS]) {
                    std::cout << "kurtosis increased" << std::endl;
                    vector[feature + (KURTOSIS * NUMBER_OF_FEATURES)] = song->feature_blocks->at(block)->kurtosis[feature];
                }
            }
        }
    }
}

void normalise(std::vector<Song *> songs, float* vector)
{
    for (unsigned int i = 0; i < songs.size(); i++) {
        Song* song = songs.at(i);
        
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

