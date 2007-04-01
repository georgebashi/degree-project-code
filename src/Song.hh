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
        
        std::string filename;
        FeatureGroup* song_features;
        std::vector<FeatureGroup *>* feature_blocks;
        
    private:
        Song() {};
};

FeatureGroup* readFeatureGroup(std::ifstream* input);
void writeFeatureGroup(std::ofstream* output, FeatureGroup* feature_group);
float compareFeatureGroup(FeatureGroup* a, FeatureGroup* b, float* weights);

#endif
