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
#include "snd.h"
#include "analysis.h"
#include "output.h"

const struct poptOption options[] = {
        {"print-audio", 'a', POPT_ARG_VAL, &print_data, OPT_PRINT_AUDIO, "Output the audio data read from the file (watch out, lots of output)", NULL},
        {"print-window", 'w', POPT_ARG_VAL, &print_data, OPT_PRINT_WINDOW, "Output the first analysis window", NULL},
        {"print-fft", 'f', POPT_ARG_VAL, &print_data, OPT_PRINT_FFT, "Output the FFT data from the first analysis window", NULL},
        {"reprofile", 'r', POPT_ARG_NONE, &reprofile, 0, "Reprofile the machine (overwrite .wisdom)", NULL},
        {"version", 'V', POPT_ARG_NONE, &display_version, 0, "Display version and compile flags", NULL},
        POPT_AUTOHELP
        POPT_TABLEEND
};

int main(int argc, const char *argv[])
{
    // read options
    poptContext context = poptGetContext("extractor", argc, argv, options, 0);
    poptSetOtherOptionHelp(context, "[OPTION...] file.wav");

    // parse
    print_data = 0;
    display_version = 0;
    poptGetNextOpt(context);
    
    // need at least 1 file arg
    if (argc == 1) {
        poptPrintUsage(context, stdout, 0);
        exit(EXIT_SUCCESS);
    }
    
    if (display_version) {
        // FIXME: add svn tag
        printf("unversioned\n");
        exit(EXIT_SUCCESS);
    }
    
    // FIXME: this is actually const char *
    // but would i ever change the argument text? ;)
    char *file = (char *) poptGetArg(context);

    // open specified file
    open_sf(file);
    
    // create fft buffer
    float *sf_read_buffer = malloc(sizeof(float) * STEP_SIZE);
    float *window = malloc(sizeof(float) * WINDOW_SIZE);
    float *fft_output = fftwf_malloc(sizeof(float) * WINDOW_SIZE);

    fftwf_plan plan = fft_setup(fft_output);
    
    // read one window into the end of the fft buffer, ready to be shifted up
    read_window(sf_read_buffer, STEP_SIZE);
    memcpy(&window[STEP_SIZE], sf_read_buffer, STEP_SIZE * sizeof(float));
    
    float *features = malloc(ANALYSIS_OUTPUTS * sizeof(float));
    
    while (read_window(sf_read_buffer, STEP_SIZE)) {
        memcpy(window, &window[STEP_SIZE], STEP_SIZE * sizeof(float));
        memcpy(&window[STEP_SIZE], sf_read_buffer, STEP_SIZE * sizeof(float));
        
        if (print_data == OPT_PRINT_AUDIO) {
            int i;
            for (i = 0; i < WINDOW_SIZE; i++) {
                print_csv_data(stdout, 1, "\"Audio\"", sf_read_buffer[i]);
            }
            exit(EXIT_SUCCESS);
        }
        
        fft_do_window(&plan, window);
        fft_fix_output(fft_output);
        
        if (print_data == OPT_PRINT_FFT) {
            int i;
            for (i = 0; i < BINS; i++) {
                print_csv_data(stdout, 1, "\"FFT Output (first window)\"", fft_output[i]);
            }
            exit(EXIT_SUCCESS);
        }
        float sc = spectral_centroid(fft_output);
        float si = spectral_intensity(fft_output);
        float ss = spectral_smoothness(fft_output);
        float lr = linear_regression(fft_output);
        
        float zcr = zero_crossing_rate(sf_read_buffer);
        
        print_csv_data(
            stdout,
            5,
            "\"Intensity\",\"Centroid\",\"Smoothness\",\"Linear Regression (gradient)\",\"Zero Crossing Rate\"",
            si,
            sc,
            ss,
            lr,
            zcr
        );
    }

    fft_stop();
    close_sf();
    free(features);
    free(sf_read_buffer);
    free(window);
    free(fft_output);
    poptFreeContext(context);
    return EXIT_SUCCESS;
}


