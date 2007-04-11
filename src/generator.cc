// $Id$

#include <vector>
#include <fnmatch.h>
#include <popt.h>
#include <string>

#include "common.hh"
#include "Features.hh"
#include "Song.hh"
#include "SongSet.hh"
#include "generator.hh"


Song* key;
float* weights;
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
    
    float comparison_weights[32] = {
                                       1, 1, 1, 1,
                                       1, 1, 1, 1,
                                       1, 1, 1, 1,
                                       1, 1, 1, 1,
                                       1, 1, 1, 1,
                                       1, 1, 1, 1,
                                       1, 1, 1, 1,
                                       1, 1, 1, 1
                                   };
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
            if (verbose) {
                std::cout << song_vectors.at(i)->filename << " (" << key->compare(song_vectors.at(i), weights, comparison_function) << ")" << std::endl;
            } else {
                std::cout << song_vectors.at(i)->filename.substr(0, song_vectors.at(i)->filename.length() - 4) << std::endl;
            }
        }
    } else if (interpolate) {
        // parse options
        int repeated_artist = REPEATED_ARTIST_LOCAL;
        if (strcasecmp(repeated_artist_str, "neighbour") == 0) {
            repeated_artist = REPEATED_ARTIST_NEIGHBOUR;
        } else if (strcasecmp(repeated_artist_str, "none") == 0) {
            repeated_artist = REPEATED_ARTIST_NONE;
        }
        
        
        std::vector<Song *> keys;
        std::vector<Song *> playlist;
        // find the key songs, removing from the song_vectors list as we go
        int i = 0;
        while (key_songs[i] != NULL) {
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
            if (next_song == NULL) { continue; }
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
        
        // print out the keys
        if (verbose) {
            std::cout << "Keys:" << std::endl;
            for (unsigned int i = 0; i < keys.size(); i++) {
                std::cout << keys.at(i)->filename << std::endl;
            }
        }
        
        // loop over each key song
        for (unsigned int i = 0; i < keys.size() - 1; i++) {
            if (verbose) { std::cout << "Key: " << i << std::endl; }
            Song* key = keys.at(i);
            playlist.push_back(key);
            for (int j = 0; j < interpolate; j++) {
                if (verbose) { std::cout << "Interpolating: " << j << std::endl; }
                FeatureGroup desired(key->song_features, keys.at(i + 1)->song_features, ((float)(j + 1) / (interpolate + 1)));
                
                // find best match
                float best_dist = 99999;
                Song* best_match = NULL;
                unsigned int match_index = 0;
                for (unsigned int k = 0; k < song_vectors.size(); k++) {
                    // evaluate if track is best match
                    Song* possible = song_vectors.at(k);
                    
                    if (repeated_artist == REPEATED_ARTIST_NEIGHBOUR && playlist.back()->get_artist() == possible->get_artist()) {
                        continue;
                    } else if (repeated_artist == REPEATED_ARTIST_LOCAL) {
                        bool skip = false;
                        for (unsigned int l = playlist.size() - j; l < playlist.size(); l++) {
                            if (playlist.at(l)->get_artist() == possible->get_artist()) {
                                skip = true;
                                break;
                            }
                        }
                        if (skip == true) { continue; }
                    }
                    
                    float score = possible->song_features->compare(&desired, weights);
                    if (score < best_dist) {
                        best_dist = score;
                        best_match = possible;
                        match_index = k;
                    }
                }
                // add to playlist and remove from available songs
                playlist.push_back(best_match);
                if (song_vectors.size() < match_index + 1) { break; }
                song_vectors.erase(song_vectors.begin() + match_index);
                if (repeated_artist == REPEATED_ARTIST_NONE) {
                    for (unsigned int k = 0; k < song_vectors.size(); k++) {
                        if (song_vectors.at(k)->get_artist() == best_match->get_artist()) {
                            song_vectors.erase(song_vectors.begin() + k);
                            k--;
                        }
                    }
                }
                if (song_vectors.size() == 0) {
                    break;
                }
            }
        }
        playlist.push_back(keys.back());
        
        std::vector<Song*>::iterator it = playlist.begin();
        while (it != playlist.end()) {
            std::cout << (*it)->filename << std::endl;
            it++;
        }
        
    }
    
    poptFreeContext(context);
    return EXIT_SUCCESS;
}

bool song_cmp(Song* s1, Song* s2)
{
    return key->compare(s1, weights, comparison_function) < key->compare(s2, weights, comparison_function);
}

