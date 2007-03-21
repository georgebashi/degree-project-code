#ifndef SONG_H
#define SONG_H

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

class Song
{
    public:
    
        Song(std::string filename);
        Song(std::string path, std::string filename);
        ~Song();
        
        void addBlock(FeatureGroup* block);
        void setSongFeatures(FeatureGroup* song_features);
        void write(std::ofstream* output);
        float compare(Song* other);
        
        std::string filename;
        FeatureGroup* song_features;
        std::vector<FeatureGroup *>* feature_blocks;
        
    private:
        Song() {}};
        
FeatureGroup* readFeatureGroup(std::ifstream* input);
void writeFeatureGroup(std::ofstream* output, FeatureGroup* feature_group);
float compareFeatureGroup(FeatureGroup* a, FeatureGroup* b);
#endif

