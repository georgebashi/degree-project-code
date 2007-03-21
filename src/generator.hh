#ifndef GENERATOR_H
#define GENERATOR_H

int display_version;
char* dir = NULL;


const struct poptOption options[] =
    {
        {"version", 'V', POPT_ARG_NONE, &display_version, 0, "Display version and compile flags", NULL},
        {"dir", 'd', POPT_ARG_STRING, &dir, 0, "Music folder", NULL},
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    
extern int main(int argc, const char *argv[]);
void getNormalisationVector(std::vector<Song *> songs, float* vector);
void normalise(std::vector<Song *> songs, float* vector);
#endif

