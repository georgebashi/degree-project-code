// $Id$

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
    setbuf(stdout, NULL);
    
    // read options
    poptContext context = poptGetContext("extractor", argc, argv, options, 0);
    poptSetOtherOptionHelp(context, "[OPTION...] file.mp3 [file.mp3...]");
    
    // parse
    display_version = 0;
    poptGetNextOpt(context);
    
    // need at least 1 file arg
    if (argc == 1) {
        poptPrintUsage(context, stdout, 0);
        exit(EXIT_SUCCESS);
    }
    
    if (display_version) {
        std::cout << "Feature Extractor: $Revision$" << std::endl;
        exit(EXIT_SUCCESS);
    }
    // get file list
    const char ** files = poptGetArgs(context);
    
    // create fft buffer
    float *wav_data = new float[STEP_SIZE];
    float *window = new float[WINDOW_SIZE];
    float *fft_output = (float *) fftwf_malloc(sizeof(float) * WINDOW_SIZE);
    
    
    
    FFT* fft = new FFT(fft_output);
    
    unsigned int i = 0;
    const char* file;
    while ((file = files[i++]) != NULL) {
        // open specified file
        std::string filename(poptGetArg(context));
        std::cout << filename;
        
        struct stat file_stat;
        if (stat((filename + ".vec").c_str(), &file_stat) == 0) {
            std::cout << ": skipping..." << std::endl;
            continue;
        }
        
        SoundFile* wav;
        try {
            wav = new SoundFile(filename);
        } catch (std::string& error) {
            std::cout << ", read error: " << error << std::endl;
            continue;
        }
        std::cout << ", analyse";
        Song next_song(filename);
        
        memset(wav_data, 0, STEP_SIZE * sizeof(float));
        memset(window, 0, WINDOW_SIZE * sizeof(float));
        
        // read one window into the end of the fft buffer, ready to be shifted up
        wav->read(wav_data, STEP_SIZE);
        memcpy(&window[STEP_SIZE], wav_data, STEP_SIZE * sizeof(float));
        
        FeatureExtractor extractor;
        std::vector<FeatureSet*>* current_song_feature_sets = new std::vector<FeatureSet*>();
        std::vector<FeatureSet*>* current_block_feature_sets = new std::vector<FeatureSet*>();
        int window_number = 0;
        
        while (wav->read(wav_data, STEP_SIZE)) {
            // move the next step into the window
            memcpy(window, &window[STEP_SIZE], STEP_SIZE * sizeof(float));
            memcpy(&window[STEP_SIZE], wav_data, STEP_SIZE * sizeof(float));
            
            // do the fft
            fft->do_window(window);
            fft->fix_output(fft_output);
            
            // get the features
            FeatureSet* new_features = extractor.process(window, fft_output);
            current_song_feature_sets->push_back(new_features);
            current_block_feature_sets->push_back(new_features);
            
            // make block
            if (window_number == WINDOWS_PER_BLOCK - 1) {
                next_song.addBlock(new FeatureGroup(current_block_feature_sets));
                delete current_block_feature_sets;
                current_block_feature_sets = new std::vector<FeatureSet*>();
                window_number = 0;
            } else {
                window_number++;
            }
        }
        next_song.feature_blocks->pop_back();
        next_song.feature_blocks->erase(next_song.feature_blocks->begin());
        next_song.setSongFeatures(new FeatureGroup(current_song_feature_sets));
        std::cout << ", done (" << next_song.feature_blocks->size() << " blocks)" << std::endl;
        
        std::ofstream ofs((next_song.filename + ".vec").c_str(), std::ios::binary);
        next_song.write(&ofs);
        
        // clean up...
        // wav file
        delete wav;
        
        //
        for (unsigned int i = 0; i < current_song_feature_sets->size(); i++) {
            delete current_song_feature_sets->at(i);
        }
        current_song_feature_sets->clear();
        delete current_song_feature_sets;
        
        delete current_block_feature_sets;
    }
    
    poptFreeContext(context);
    delete fft;
    delete [] wav_data;
    delete [] window;
    free(fft_output);
    
    return EXIT_SUCCESS;
}

