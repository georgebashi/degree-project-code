// $Id$

#include <vector>
#include <fstream>
#include <popt.h>
#include <string>

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
    const char ** key_songs = poptGetArgs(context);
    
    SongSet song_vectors(dir);
    song_vectors.normalise();
    
    if (!song_vectors.size()) {
        std::cout << "No songs analysed yet!" << std::endl;
        exit(0);
    }
    
    std::cout << song_vectors.size() << " songs loaded" << std::endl;
    
    std::string line, key, similar, not_similar;
    std::ifstream results_reader;
    char delim = '*';
    results_reader.open(results_file);
    
    while (getline(results_reader, line, delim)) {
        if (key.empty()) {
            key = line;
        } else if (similar.empty()) {
            similar = line;
            delim = '\n';
        } else {
            not_similar = line;
            trim(key);
            trim(similar);
            trim(not_similar);
            printf("key: %s\nsimilar: %s\nnot_similar: %s\n", key.c_str(), similar.c_str(), not_similar.c_str());
            
            // DO THINGS
            
            // get ready for next set
            delim = '*';
            key.clear();
            similar.clear();
            not_similar.clear();
        }
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
