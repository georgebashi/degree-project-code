#ifndef SOUNDFILE_H
#define SOUNDFILE_H

#include <sndfile.h>

class SoundFile
{
public:
    SoundFile(std::string filename);
    ~SoundFile();
    int read(float *buffer, int n);
    //char* get_md5();
private:
    SNDFILE *sf_in;
    SF_INFO *sfinfo;
    //MD5_CTX md5_context;
};

#endif
