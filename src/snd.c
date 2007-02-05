#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

#include "common.h"
#include "snd.h"

SNDFILE *sf_in;
SF_INFO *sfinfo;

void open_sf(char *filename)
{
    sfinfo = malloc(sizeof(SF_INFO));
    
    if(!(sf_in = sf_open(filename, SFM_READ, sfinfo)))
    {
        printf("Error opening input file.\n");
        exit(EXIT_FAILURE);
    }
}

void close_sf()
{
    sf_close(sf_in);
    free(sfinfo);
}

/*
    read a specified amount from the file and convert to mono
*/
int read_window(float* output, int n)
{
    int read_length = n * sfinfo->channels;
    float *in_buffer = malloc(sizeof(float) * read_length);
    
    // couldn't read n samples
    if (!sf_readf_float(sf_in, in_buffer, n)) {
            return 0;
    }
    
    int i, out_pos = 0;
    for (i = 0; i < read_length; i++) {
        output[out_pos] += in_buffer[i];
        
        if (i % sfinfo->channels == 0) {
            output[out_pos++] /= sfinfo->channels;
        }
    }
    
    free(in_buffer);
    return 1;
}
