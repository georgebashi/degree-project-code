// $Id$

/*
Playlist Generator
 
*/

#include <vector>
#include <fnmatch.h>
#include <popt.h>
#include <string>
#include <cmath>

#include "common.hh"
#include "Features.hh"
#include "Song.hh"
#include "SongSet.hh"
#include "Playlist.hh"
#include "generator.hh"


Song* key;
float (*weights)[NUMBER_OF_AGGREGATE_STATS];
int comparison_function;

int main(int argc, const char *argv[])
{
    // read options
    poptContext context = poptGetContext("generator", argc, argv, options, 0);
    poptSetOtherOptionHelp(context, "[OPTION...] file.mp3");
    
    // parse
    display_version = 0;
    verbose = 0;
    n_similar = 0;
    interpolate = 10;
    dir = ".";
    comp_str = "ordered";
    repeated_artist_str = "local";
    poptGetNextOpt(context);
    
    // need at least 1 file arg
    if (argc == 1) {
        poptPrintUsage(context, stdout, 0);
        exit(EXIT_SUCCESS);
    }
    
    if (display_version) {
        std::cout << "Playlist Generator: $Revision$" << std::endl;
        exit(EXIT_SUCCESS);
    }
    
    // get file list
    const char ** key_songs = poptGetArgs(context);
    
    SongSet song_vectors(dir);
    song_vectors.normalise();
    
    if (!song_vectors.size()) {
        std::cout << "No songs analysed yet!" << std::endl;
        exit(0);
    }
    
    if (verbose) {
        std::cout << song_vectors.size() << " songs loaded" << std::endl;
    }
    
    float comparison_weights[NUMBER_OF_FEATURES][NUMBER_OF_AGGREGATE_STATS] = {
{ 0.613092, 0.205134, 0.533735, 0.110462}, 
{ 0.306893, 0.132636, 0.453016, 0.0505156}, 
{ 0.321113, 0.183988, 0.485224, 0.0169958}, 
{ 0.459398, 0.206291, 0.48236, 0.260304}, 
{ 0.279731, 0.121066, 0.449003, 0.0465693}, 
{ 0.677096, 1, 0.786617, 0.545458}, 
{ 0.997773, 0.680246, 0.306549, 0.0242729}, 
{ 0.718418, 0.303971, 0.29711, 0}            };
    weights = comparison_weights;
    
    if (n_similar) {
        if (key_songs == NULL) {
            std::cout << "Please specify a key song with -s" << std::endl;
            exit(0);
        }
        
        comparison_function = ORDERED;
        if (strcasecmp(comp_str, "ordered-area") == 0) {
            comparison_function = ORDERED_AREA;
        } else if (strcasecmp(comp_str, "sorted") == 0) {
            comparison_function = SORTED;
        } else if (strcasecmp(comp_str, "total") == 0) {
            comparison_function = TOTAL;
        }
        
        for (unsigned int i = 0; i < song_vectors.size(); i++) {
            if (fnmatch(
                        (std::string("*") + std::string(key_songs[0]) + std::string("*")).c_str(),
                        song_vectors.at(i)->filename.c_str(),
                        0)
                    == 0) {
                key = song_vectors.at(i);
                break;
            }
        }
        
        std::sort(song_vectors.begin(), song_vectors.end(), song_cmp);
        
        for (int i = 0; i < n_similar; i++) {
        	float score = key->compare(song_vectors.at(i), weights, comparison_function);
        	if (!std::isnormal(score)) { n_similar++; continue; }
            if (verbose) {
                std::cout << song_vectors.at(i)->filename.substr(0, song_vectors.at(i)->filename.length() - 4) << " (" << score << ")" << std::endl;
            } else {
                std::cout << song_vectors.at(i)->filename.substr(0, song_vectors.at(i)->filename.length() - 4) << std::endl;
            }
        }
    } else if (interpolate) {
        if (key_songs == NULL) {
            std::cout << "Please specify some key songs" << std::endl;
            exit(0);
        }
        
        // parse options
        int repeated_artist = REPEATED_ARTIST_LOCAL;
        if (strcasecmp(repeated_artist_str, "neighbour") == 0) {
            repeated_artist = REPEATED_ARTIST_NEIGHBOUR;
        } else if (strcasecmp(repeated_artist_str, "none") == 0) {
            repeated_artist = REPEATED_ARTIST_NONE;
        }
        
        
        std::vector<Song *> keys;
        // find the key songs, removing from the song_vectors list as we go
        int i = 0;
        while (key_songs[i] != NULL) {
            //std::cout << key_songs[i] << std::endl;
            Song* next_song = NULL;
            unsigned int index = 0;
            for (unsigned int j = song_vectors.size(); j--;) {
                if (song_vectors.at(j)->filename == std::string(key_songs[i])) {
                    next_song = song_vectors.at(j);
                    index = j;
                    break;
                }
            }
            i++;
            if (next_song == NULL) { std::cout << "Couldn't find, skipping!" << std::endl; continue; }
            keys.push_back(next_song);
            song_vectors.erase(song_vectors.begin() + index);
            if (repeated_artist == REPEATED_ARTIST_NONE) {
                for (unsigned int j = 0; j < song_vectors.size(); j++) {
                    if (song_vectors.at(j)->get_artist() == next_song->get_artist()) {
                        song_vectors.erase(song_vectors.begin() + j);
                        j--;
                    }
                }
            }
        }
        // generate a playlist!
        Playlist playlist(&song_vectors, keys, interpolate, weights, comparison_function);
        playlist.print();
    }
    
    poptFreeContext(context);
    return EXIT_SUCCESS;
}

bool song_cmp(Song* s1, Song* s2)
{
    return key->compare(s1, weights, comparison_function) < key->compare(s2, weights, comparison_function);
}

