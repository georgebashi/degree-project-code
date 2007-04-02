// $Id$

#ifndef SONG_HH
#define SONG_HH

#include <string>
#include <iostream>
#include <vector>
#include "Features.hh"
#include "Song.hh"

#ifdef DEBUG
#define CHECK(x) if (!std::isnormal(x)) { /*std::cout << "!"; return 0;*/ }
// #define CHECK(x) (assert(fpclassify(x) == FP_NORMAL))
#else
#define CHECK(x)
#endif

#define WEIGHT(F,S) (weights[(((F) * (NUMBER_OF_AGGREGATE_STATS)) + (S))])

#define ORDERED 0
#define ORDERED_AREA 1
#define SORTED 2
#define TOTAL 3

class Song
{
    public:
    
        Song(std::string filename);
        Song(std::string path, std::string filename);
        bool operator == (const Song& other)
        {
            return filename == other.filename;
        };
        ~Song();
        
        
        bool has_nan();
        void addBlock(FeatureGroup* block);
        void setSongFeatures(FeatureGroup* song_features);
        void write(std::ofstream* output);
        float compare(Song* other);
        float compare(Song* other, float* weights, int comparison_function);
        std::string filename;
        FeatureGroup* song_features;
        std::vector<FeatureGroup *>* feature_blocks;
        
    private:
        Song() {};
};

bool fg_cmp(FeatureGroup* fg1, FeatureGroup* fg2);
FeatureGroup* readFeatureGroup(std::ifstream* input);
void writeFeatureGroup(std::ofstream* output, FeatureGroup* feature_group);
float compareFeatureGroup(FeatureGroup* a, FeatureGroup* b, float* weights);

#endif
