// $Id$

#include <vector>
#include <fstream>
#include <popt.h>
#include <string>
#include <cmath>
#include <map>

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
    
    float weights[NUMBER_OF_FEATURES][NUMBER_OF_AGGREGATE_STATS] = {
                { 0, 0, 0, 0 },
                { 0, 0, 0, 0 },
                { 0, 0, 0, 0 },
                { 0, 0, 0, 0 },
                { 0, 0, 0, 0 },
                { 0, 0, 0, 0 },
                { 0, 0, 0, 0 },
                { 0, 0, 0, 0 }
            };
            
    SongSet song_vectors(dir);
    song_vectors.normalise();
    
    if (!song_vectors.size()) {
        std::cout << "No songs analysed yet!" << std::endl;
        exit(0);
    }
    
    std::cout << song_vectors.size() << " songs loaded" << std::endl;
    
    std::map<std::string, std::vector<Song *> > albums;
    for (unsigned int i = 0; i < song_vectors.size(); i++) {
        std::string new_album = song_vectors.at(i)->get_album();
        std::map<std::string, std::vector<Song *> >::iterator other_tracks = albums.find(new_album);
        if (other_tracks == albums.end()) {
            std::vector<Song *> new_vector;
            new_vector.push_back(song_vectors.at(i));
            albums[new_album] = new_vector;
        } else {
            other_tracks->second.push_back(song_vectors.at(i));
        }
    }
    
    for(
        std::map<std::string, std::vector<Song *> >::iterator iter = albums.begin();
        iter != albums.end();
        iter++
    ) {
        for (unsigned int i = 0; i < iter->second.size() - 1; i++) {
            for (unsigned int j = i + 1; j < iter->second.size(); j++) {
                for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
                    for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
                        weights[feature][stat] +=
                             fabsf(iter->second.at(i)->song_features->features[feature][stat] -
                             iter->second.at(j)->song_features->features[feature][stat]) /
                            iter->second.size()
                            ;
                    }
                }
            }
        }
    }
    
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
            
            Song* song_similar_a = song_vectors.get_by_filename(similar_a + ".vec");
            Song* song_similar_b = song_vectors.get_by_filename(similar_b + ".vec");
            Song* song_not_similar = song_vectors.get_by_filename(not_similar + ".vec");
            
            if (song_similar_a == NULL | song_similar_b == NULL | song_not_similar == NULL) {
                continue;
            }
            
            for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
                for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
                    weights[feature][stat] +=
                        fabsf(song_similar_a->song_features->features[feature][stat] -
                        song_similar_b->song_features->features[feature][stat]);
                }
            }
            
            // get ready for next set
            delim = '*';
            similar_a.clear();
            similar_b.clear();
            not_similar.clear();
        }
    }
    
    // normalise the weights
    float max = weights[0][0];
    float min = weights[0][0];
    for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
            if (weights[feature][stat] > max) {
                max = weights[feature][stat];
            }
            if (weights[feature][stat] < min) {
                min = weights[feature][stat];
            }
        }
    }
    for (int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        std::cout << "{ ";
        for (int stat = 0; stat < NUMBER_OF_AGGREGATE_STATS; stat++) {
            weights[feature][stat] = (weights[feature][stat] - min) / (max - min);
            std::cout << weights[feature][stat];
            if (stat != NUMBER_OF_AGGREGATE_STATS - 1) {
                std::cout << ", ";
            }
        }
        if (feature != NUMBER_OF_FEATURES - 1) {
            std::cout << "}, " << std::endl;
        } else {
            std::cout << "}" << std::endl;
        }
    }
    
    return 0;
}

/*
 * The following code is used to remove leading and trailing space from
 * a string, and was taken from a tutorial at:
 * http://www.codeproject.com/vcpp/stl/stdstringtrim.asp
 */
void trim(std::string& str)
{
    std::string::size_type pos = str.find_last_not_of(' ');
    if (pos != std::string::npos) {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if (pos != std::string::npos) str.erase(0, pos);
    } else str.erase(str.begin(), str.end());
}
