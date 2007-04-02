// $Id$

#ifndef SONG_SET_HH
#define SONG_SET_HH

class SongSet : public std::vector<Song*>
{
    public:
        SongSet(std::string dir);
        std::string dir;
        void normalise();
        Song* get_by_filename(std::string filename);
        
    private:
        SongSet() {};
};

int read_song_files(const char *fpath, const struct stat *sb, int typeflag);

#endif
