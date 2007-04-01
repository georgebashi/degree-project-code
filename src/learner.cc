
#include <vector>
#include <fnmatch.h>
#include <popt.h>
#include <string>

#include "common.hh"
#include "Features.hh"
#include "Song.hh"
#include "SongSet.hh"
#include "generator.hh"


int main(int argc, const char *argv[])
{
    // read options
    poptContext context = poptGetContext("generator", argc, argv, options, 0);
    poptSetOtherOptionHelp(context, "[OPTION...]");
    
    // parse
    display_version = 0;
    dir = ".";
    results_file = "results.out";
    
    poptGetNextOpt(context);
    
    if (display_version) {
        std::cout << "Weight Trainer: svn-$Revision$" << std::endl;
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
    
    return 0;
}
