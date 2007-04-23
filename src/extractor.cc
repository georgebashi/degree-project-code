// $Id$

/*
Feature Extractor
Reads in audio files specified on the command-line, computes features and stores these into .vec files.
*/

#include <popt.h>
#include <sys/stat.h>
#include <fstream>

#include "common.hh"
#include "FFT.hh"
#include "SoundFile.hh"
#include "Features.hh"
#include "Song.hh"

int display_version;

const struct poptOption options[] =
    {
        {"version", 'V', POPT_ARG_NONE, &display_version, 0, "Display version", NULL
        },
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    
int main(int argc, const char *argv[])
{
    // Don't let the command-line output be buffered (or the pretty output won't work)
    setbuf(stdout, NULL);
    
    // Read options
    poptContext context = poptGetContext("extractor", argc, argv, options, 0);
    poptSetOtherOptionHelp(context, "[OPTION...] file.mp3 [file.mp3...]");
    
    // Parse them
    display_version = 0;
    poptGetNextOpt(context);
    
    // Check we have at least one file argument
    if (argc == 1) {
        poptPrintUsage(context, stdout, 0);
        exit(EXIT_SUCCESS);
    }
    
    // Display version if asked
    if (display_version) {
        std::cout << "Feature Extractor: $Revision$" << std::endl;
        exit(EXIT_SUCCESS);
    }
    
    // Get file list
    const char ** files = poptGetArgs(context);
    
    // Create FFT memory buffer
    float *wav_data = new float[STEP_SIZE];
    float *window = new float[WINDOW_SIZE];
    float *fft_output = (float *) fftwf_malloc(sizeof(float) * WINDOW_SIZE);
    
    
    // Set up the FFT for processing
    FFT* fft = new FFT(fft_output);
    
    // Loop over and process each file specified
    unsigned int i = 0;
    const char* file;
    while ((file = files[i++]) != NULL) {
        // Display the filename
        std::string filename(poptGetArg(context));
        std::cout << filename;
        
        // Skip the file if it wasn't found or it can't be opened
        struct stat file_stat;
        if (stat((filename + ".vec").c_str(), &file_stat) == 0) {
            std::cout << ": skipping..." << std::endl;
            continue;
        }
        
        // Open the file
        SoundFile* wav;
        try {
            wav = new SoundFile(filename);
        } catch (std::string& error) {
            std::cout << ", read error: " << error << std::endl;
            continue;
        }
        
        // Create a blank Song for storage of the features
        std::cout << ", analyse";
        Song next_song(filename);
        
        // Clear the memory which holds the audio data and FFT output
        memset(wav_data, 0, STEP_SIZE * sizeof(float));
        memset(window, 0, WINDOW_SIZE * sizeof(float));
        
        // Read one window into the end of the FFT buffer, ready to be shifted up
        wav->read(wav_data, STEP_SIZE);
        memcpy(&window[STEP_SIZE], wav_data, STEP_SIZE * sizeof(float));
        
        // Create the vectors to hold processed audio windows
        FeatureExtractor extractor;
        std::vector<FeatureSet*>* current_song_feature_sets = new std::vector<FeatureSet*>();
        std::vector<FeatureSet*>* current_block_feature_sets = new std::vector<FeatureSet*>();
        int window_number = 0;
        
        // Loop over each window in the audio file
        while (wav->read(wav_data, STEP_SIZE)) {
            // Move the next step into the window
            memcpy(window, &window[STEP_SIZE], STEP_SIZE * sizeof(float));
            memcpy(&window[STEP_SIZE], wav_data, STEP_SIZE * sizeof(float));
            
            // Compute the FFT
            fft->do_window(window);
            fft->fix_output(fft_output);
            
            // Get the features for this window
            FeatureSet* new_features = extractor.process(window, fft_output);
            current_song_feature_sets->push_back(new_features);
            current_block_feature_sets->push_back(new_features);
            
            // Calculate block-leve statistics
            if (window_number == WINDOWS_PER_BLOCK - 1) {
                next_song.addBlock(new FeatureGroup(current_block_feature_sets));
                delete current_block_feature_sets;
                current_block_feature_sets = new std::vector<FeatureSet*>();
                window_number = 0;
            } else {
                window_number++;
            }
        }
        // Finished this song now, tidy up
        next_song.feature_blocks->pop_back();
        next_song.feature_blocks->erase(next_song.feature_blocks->begin());
        // Calculate the song features
        next_song.setSongFeatures(new FeatureGroup(current_song_feature_sets));
        // Output a message
        std::cout << ", done (" << next_song.feature_blocks->size() << " blocks)" << std::endl;
        
        // Write out the .vec file
        std::ofstream ofs((next_song.filename + ".vec").c_str(), std::ios::binary);
        next_song.write(&ofs);
        
        // Clean up the memory used
        delete wav;
        for (unsigned int i = 0; i < current_song_feature_sets->size(); i++) {
            delete current_song_feature_sets->at(i);
        }
        current_song_feature_sets->clear();
        delete current_song_feature_sets;
        delete current_block_feature_sets;
    }
    
    // Finished all the songs, tidy up.
    poptFreeContext(context);
    delete fft;
    delete [] wav_data;
    delete [] window;
    free(fft_output);
    
    return EXIT_SUCCESS;
    // yay!
}

