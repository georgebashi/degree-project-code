// $Id$

#include <string>
#include <sndfile.h>
#include <iostream>
#include <stdlib.h>

#include "common.hh"
#include "SoundFile.hh"


SoundFile::SoundFile(std::string filename)
{
    // find name for temp file
    tmp_filename = tempnam("/tmp", "dec-");
    // fire up gstreamer
    std::cout << ": decode";
    std::string cmdline;
    char *pwd = get_current_dir_name();
    
    cmdline.append("/usr/bin/gst-launch-0.10 filesrc location=\"").append(pwd).append("/").append(filename);
    cmdline.append("\" ! decodebin ! audioconvert ! audio/x-raw-int,channels=1,rate=44100 ! wavenc ! filesink location=");
    cmdline.append(tmp_filename).append(" > /dev/null 2>&1");
    
    free(pwd);
    
    system(cmdline.c_str());
    
    sfinfo = (SF_INFO *) calloc(1, sizeof(SF_INFO));
    if ((sf_in = sf_open(tmp_filename, SFM_READ, sfinfo)) == NULL) {
        throw std::string(sf_strerror(sf_in));
    }
}

SoundFile::~SoundFile()
{
    sf_close(sf_in);
    free(sfinfo);
    unlink(tmp_filename);
    free(tmp_filename);
}

int SoundFile::read(float* buffer, int n)
{
    if (sf_readf_float(sf_in, buffer, n) != n) {
        return 0;
    }
    
    return n;
}

