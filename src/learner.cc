// $Id$

#include <vector>
#include <fstream>
#include <popt.h>
#include <string>
#include <cmath>

#include "common.hh"
#include "Features.hh"
#include "Song.hh"
#include "SongSet.hh"
#include "learner.hh"


int main(int argc, const char *argv[])
{
    // read options
    poptContext context = poptGetContext("learner", argc, argv, options, 0);
    poptSetOtherOptionHelp(context, "[OPTION...]");
    
    // parse
    display_version = 0;
    dir = ".";
    results_file = "results.txt";
    
    poptGetNextOpt(context);
    
    if (display_version) {
        std::cout << "Weight Trainer: $Revision$" << std::endl;
        exit(EXIT_SUCCESS);
    }
    
    // get file list
    //const char ** key_songs = poptGetArgs(context);
    
    float weights[32] = {
                            0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0,
                            0, 0
                        };
                        
    SongSet song_vectors(dir);
    song_vectors.normalise();
    
    if (!song_vectors.size()) {
        std::cout << "No songs analysed yet!" << std::endl;
        exit(0);
    }
    
    std::cout << song_vectors.size() << " songs loaded" << std::endl;
    
    std::string line, similar_a, similar_b, not_similar;
    std::ifstream results_reader;
    char delim = '*';
    results_reader.open(results_file);
    
    while (getline(results_reader, line, delim)) {
        if (similar_a.empty()) {
            similar_a = line;
        } else if (similar_b.empty()) {
            similar_b = line;
            delim = '\n';
        } else {
            not_similar = line;
            trim(similar_a);
            trim(similar_b);
            trim(not_similar);
            //printf("key: %s\nsimilar: %s\nnot_similar: %s\n", key.c_str(), similar.c_str(), not_similar.c_str());
            
            // DO THINGS
            Song* song_similar_a = song_vectors.get_by_filename(similar_a + ".vec");
            Song* song_similar_b = song_vectors.get_by_filename(similar_b + ".vec");
            Song* song_not_similar = song_vectors.get_by_filename(not_similar + ".vec");
            
            if (song_similar_a == NULL | song_similar_b == NULL | song_not_similar == NULL) {
                //std::cerr << "Couldn't find song!" << std::endl;
                continue;
            }
            
            for (int i = 0; i < NUMBER_OF_FEATURES; i++) {
                //std::cout << song_similar_a->song_features->mean[i] << std::endl;
                WEIGHT(i, MEAN) += 1 - (fabsf(fabsf(song_similar_a->song_features->mean[i]) - fabsf(song_similar_b->song_features->mean[i])));
                WEIGHT(i, VARIANCE) += 1 - (fabsf(fabsf(song_similar_a->song_features->variance[i]) - fabsf(song_similar_b->song_features->variance[i])));
                WEIGHT(i, SKEWNESS) += 1 - (fabsf(fabsf(song_similar_a->song_features->skewness[i]) - fabsf(song_similar_b->song_features->skewness[i])));
                WEIGHT(i, KURTOSIS) += 1 - (fabsf(fabsf(song_similar_a->song_features->kurtosis[i]) - fabsf(song_similar_b->song_features->kurtosis[i])));
            }
            
            // get ready for next set
            delim = '*';
            similar_a.clear();
            similar_b.clear();
            not_similar.clear();
        }
    }
    
    // normalise the weights
    float max = 0;
    for (int i = 0; i < 32; i++) {
        if (weights[i] > max) {
            max = weights[i];
        }
    }
    for (int i = 0; i < 32; i++) {
        weights[i] /= max;
        std::cout << weights[i] << ",";
    }
    
    return 0;
}

/* from: http://www.codeproject.com/vcpp/stl/stdstringtrim.asp */
void trim(std::string& str)
{
    std::string::size_type pos = str.find_last_not_of(' ');
    if (pos != std::string::npos) {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if (pos != std::string::npos) str.erase(0, pos);
    } else str.erase(str.begin(), str.end());
}
