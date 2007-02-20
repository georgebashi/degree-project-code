
#include <string.h>
#include <sndfile.h>
#include <iostream>
#include <stdlib.h>

#include "common.hh"
#include "SoundFile.hh"


SoundFile::SoundFile(std::string filename)
{
    sfinfo = (SF_INFO *) calloc(1, sizeof(SF_INFO));
    //MD5_Init(&md5_context);
    if (!(sf_in = sf_open(filename.c_str(), SFM_READ, sfinfo))) {
        std::cout << "Error opening input file." << std::endl;
        exit(EXIT_FAILURE);
    }
}

SoundFile::~SoundFile()
{
    sf_close(sf_in);
    free(sfinfo);
}

int SoundFile::read(float* buffer, int n)
{
    if (!sf_readf_float(sf_in, buffer, n)) {
        return 0;
    }
    
    //MD5_Update(&md5_context, buffer, n);

    return n;
}

/*char* get_md5()
{
    char* md5 = new char[MD5_DIGEST_LENGTH];
    MD5_Final(md5, md5_context);
    return md5;
}*/
