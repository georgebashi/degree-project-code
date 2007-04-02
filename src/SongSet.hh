// $Id$

#ifndef SONG_SET_HH
#define SONG_SET_HH

class SongSet : public std::vector<Song*>
{
    public:
        SongSet(std::string dir);
        std::string dir;
        void normalise();
    private:
        SongSet() {};
};

int read_song_files(const char *fpath, const struct stat *sb, int typeflag);

#endif
