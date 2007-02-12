#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fftw3.h>
#include <popt.h>

#include "common.h"
#include "extractor.h"
#include "fft.h"
#include "wav.h"
#include "analysis.h"
#include "stats.h"
#include "output.h"

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
        printf("%s\n", VERSION);
        exit(EXIT_SUCCESS);
    }
    
    char *file = (char *) poptGetArg(context);
    
    poptFreeContext(context);
    
    // open specified file
    wav_open(file);
    
    // create fft buffer
    float *wav_data = malloc(sizeof(float) * STEP_SIZE);
    float *window = malloc(sizeof(float) * WINDOW_SIZE);
    float *fft_output = fftwf_malloc(sizeof(float) * WINDOW_SIZE);
    
    fftwf_plan plan = fft_setup(fft_output);
    
    // read one window into the end of the fft buffer, ready to be shifted up
    wav_read(wav_data, STEP_SIZE);
    memcpy(&window[STEP_SIZE], wav_data, STEP_SIZE * sizeof(float));
    
    int window_number = 0;
    PerceptualBlockListItem *block = malloc(sizeof(PerceptualBlockListItem));
    block->prev = NULL;
    float **block_features = malloc(sizeof(float *) * WINDOWS_PER_BLOCK);
    
    while (wav_read(wav_data, STEP_SIZE)) {
        // move the next step into the window
        memcpy(window, &window[STEP_SIZE], STEP_SIZE * sizeof(float));
        memcpy(&window[STEP_SIZE], wav_data, STEP_SIZE * sizeof(float));
        
        // do the fft
        fft_do_window(&plan, window);
        fft_fix_output(fft_output);
        
        // extract features
        block_features[window_number] = extract_features(window, fft_output);
        
        print_csv_data(
            stdout,
            ANALYSIS_OUTPUTS,
            ANALYSIS_OUTPUT_CSV_HEADER,
            block_features[window_number][0],
            block_features[window_number][1],
            block_features[window_number][2],
            block_features[window_number][3],
            block_features[window_number][4],
            block_features[window_number][5],
            block_features[window_number][6],
            block_features[window_number][7]
        );
        
        if (window_number == WINDOWS_PER_BLOCK - 1) {
            PerceptualBlockListItem *new_block = malloc(sizeof(PerceptualBlockListItem));
            new_block->block = create_perceptual_block(block_features);
            new_block->prev = block;
            block = new_block;
            window_number = 0;
            int i;
            for (i = 0; i < WINDOWS_PER_BLOCK; i++) {
                free(block_features[i]);
            }
        } else {
            window_number++;
        }
    }
    while (block->prev != NULL) {
        PerceptualBlockListItem *next = block->prev;
        free(block);
        block = next;
    }
    wav_close();
    
    // cleanup
    fft_stop();
    free(block_features);
    free(wav_data);
    free(window);
    free(fft_output);
    
    return EXIT_SUCCESS;
}
