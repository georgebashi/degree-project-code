// $Id$

#ifndef SOUNDFILE_HH
#define SOUNDFILE_HH

#include <string>
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

