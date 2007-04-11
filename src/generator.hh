// $Id$

#ifndef GENERATOR_HH
#define GENERATOR_HH

#define REPEATED_ARTIST_NONE 0
#define REPEATED_ARTIST_LOCAL 1
#define REPEATED_ARTIST_NEIGHBOUR 2


int display_version;
char* dir = NULL;
int n_similar;
int interpolate;
char* repeated_artist_str;
char* comp_str;
int verbose;

const struct poptOption options[] =
    {
        {"version", 'V', POPT_ARG_NONE, &display_version, 0, "Display version and compile flags", NULL},
        {"verbose", 'v', POPT_ARG_NONE, &verbose, 0, "Be verbose", NULL},
        {"dir", 'd', POPT_ARG_STRING, &dir, 0, "Music folder", NULL},
        {"similar", 's', POPT_ARG_INT, &n_similar, 0, "Return n most similar tracks", NULL},
        {"comparison", 'c', POPT_ARG_STRING, &comp_str, 0, "Comparison function: one of ordered, ordered-area, sorted, total", NULL},
        {"interpolate", 'n', POPT_ARG_INT, &interpolate, 0, "Number of tracks to interpolate between given key tracks", NULL},
        {"repeated-artist", 'r', POPT_ARG_STRING, &repeated_artist_str, 0, "Where to allow tracks by the same artist: one of none, local, neighbour", NULL},
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    
extern int main(int argc, const char *argv[]);
bool song_cmp(Song* s1, Song* s2);

#endif

