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
    dir = ".";
    comp_str = "ordered";
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
    
    if (n_similar) {
        if (key_songs == NULL) {
            std::cout << "Please specify a key song with -s" << std::endl;
            exit(0);
        }
        
        comparison_function = ORDERED;
        if (strcasecmp(comp_str, "ordered-area")) {
            comparison_function = ORDERED_AREA;
        } else if (strcasecmp(comp_str, "sorted")) {
            comparison_function = SORTED;
        } else if (strcasecmp(comp_str, "total")) {
            comparison_function = TOTAL;
        }
        
        for (unsigned int i = 0; i < song_vectors.size(); i++) {
            if (fnmatch(
                        (std::string("*") + std::string(key_songs[0]) + std::string("*")).c_str(),
                        song_vectors.at(i)->filename.c_str(),
                        0)
                    == 0) {
                key = song_vectors.at(i);
                //std::cout << key->filename << std::endl;
                break;
            }
        }
        
        float comparison_weights[32] = {
                                           1, 0, 0, 0,
                                           1, 0, 0, 0,
                                           1, 0, 0, 0,
                                           1, 0, 0, 0,
                                           1, 0, 0, 0,
                                           1, 0, 0, 0,
                                           1, 0, 0, 0,
                                           1, 0, 0, 0
                                       };
        weights = comparison_weights;
        
        std::sort(song_vectors.begin(), song_vectors.end(), song_cmp);
        
        for (int i = 0; i < n_similar; i++) {
            if (verbose) {
                std::cout << song_vectors.at(i)->filename << " (" << key->compare(song_vectors.at(i), weights, comparison_function) << ")" << std::endl;
            } else {
                std::cout << song_vectors.at(i)->filename.substr(0, song_vectors.at(i)->filename.length() - 4) << std::endl;
            }
        }
    }
    
    poptFreeContext(context);
    return EXIT_SUCCESS;
}

bool song_cmp(Song* s1, Song* s2)
{
    return key->compare(s1, weights, comparison_function) < key->compare(s2, weights, comparison_function);
}

