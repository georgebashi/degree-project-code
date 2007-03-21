#ifndef SOUNDFILE_H
#define SOUNDFILE_H

#include <sndfile.h>

class SoundFile
{
    public:
        SoundFile(std::string filename);
        ~SoundFile();
        int read(float *buffer, int n);
    private:
        SNDFILE *sf_in;
        SF_INFO *sfinfo;
        char *tmp_filename;
};

#endif

