#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

#include "common.h"
#include "wav.h"

SNDFILE *sf_in;
SF_INFO *sfinfo;

void wav_open(char *filename)
{
    // calloc to make sure memory is clear
    sfinfo = calloc(1, sizeof(SF_INFO));
    
    if (!(sf_in = sf_open(filename, SFM_READ, sfinfo))) {
        printf("Error opening input file.\n");
        exit(EXIT_FAILURE);
    }
}

void wav_close()
{
    sf_close(sf_in);
    free(sfinfo);
}

/*
    read a specified amount from the file
*/
int wav_read(float* output, int n)
{
    if (!sf_readf_float(sf_in, output, n)) {
        return 0;
    }
    return n;
}
