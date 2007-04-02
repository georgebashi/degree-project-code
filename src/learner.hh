// $Id$

#ifndef LEARNER_HH
#define LEARNER_HH

int display_version;
char* dir = NULL;
char* results_file = NULL;


const struct poptOption options[] =
    {
        {"version", 'V', POPT_ARG_NONE, &display_version, 0, "Display version and compile flags", NULL},
        {"dir", 'd', POPT_ARG_STRING, &dir, 0, "Music folder", NULL},
        {"results-file", 'r', POPT_ARG_STRING, &results_file, 0, "Results file (if different from \"results.txt\")", NULL},
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    
extern int main(int argc, const char *argv[]);
bool song_cmp(Song* s1, Song* s2);
void trim(std::string& str);

#endif

