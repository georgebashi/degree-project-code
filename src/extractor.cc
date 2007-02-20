#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <popt.h>
#include <vector>

#include "common.hh"
#include "FFT.hh"
#include "SoundFile.hh"
#include "Features.hh"

int display_version;

const struct poptOption options[] =
    {
        {"version", 'V', POPT_ARG_NONE, &display_version, 0, "Display version and compile flags", NULL
        },
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    
int main(int argc, const char *argv[])
{
    // read options
    poptContext context = poptGetContext("extractor", argc, argv, options, 0);
    poptSetOtherOptionHelp(context, "[OPTION...] file.wav");
    
    // parse
    display_version = 0;
    poptGetNextOpt(context);
    
    // need at least 1 file arg
    if (argc == 1) {
        poptPrintUsage(context, stdout, 0);
        exit(EXIT_SUCCESS);
    }
    
    if (display_version) {
        std::cout << VERSION << std::endl;
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
    while ((file = files[i++]) != NULL)
    {
        // open specified file
        SoundFile* wav = new SoundFile(std::string(poptGetArg(context)));
        
        memset(wav_data, 0, STEP_SIZE * sizeof(float));
        memset(window, 0, WINDOW_SIZE * sizeof(float));
        
        // read one window into the end of the fft buffer, ready to be shifted up
        wav->read(wav_data, STEP_SIZE);
        memcpy(&window[STEP_SIZE], wav_data, STEP_SIZE * sizeof(float));
        
        FeatureExtractor extractor;
        std::vector<FeatureSet*>* block_features = new std::vector<FeatureSet*>();
        std::vector<FeatureGroup*>* blocks = new std::vector<FeatureGroup*>();
        int window_number = 0;
        
        while (wav->read(wav_data, STEP_SIZE)) {
            // move the next step into the window
            memcpy(window, &window[STEP_SIZE], STEP_SIZE * sizeof(float));
            memcpy(&window[STEP_SIZE], wav_data, STEP_SIZE * sizeof(float));
            
            // do the fft
            fft->do_window(window);
            fft->fix_output(fft_output);
            
            block_features->push_back(extractor.process(window, fft_output));
            
            if (window_number == WINDOWS_PER_BLOCK - 1) {
                blocks->push_back(new FeatureGroup(block_features));
                block_features = new std::vector<FeatureSet*>();
                window_number = 0;
            } else {
                window_number++;
            }
                    
        }
        delete wav;
    }
    
    poptFreeContext(context);
    delete fft;
    delete [] wav_data;
    delete [] window;
    free(fft_output);
    
    return EXIT_SUCCESS;
}
