// $Id$

#ifndef PLAYLIST_HH
#define PLAYLIST_HH

bool playlist_song_cmp(Song* s1, Song* s2);

class PlaylistEntry
{
    public:
        PlaylistEntry(SongSet *candidiates, Song *key1, Song* key2, float dist);
        void removeArtist(std::string artist);
        Song *getBestMatch();
        float getScore();
    private:
        PlaylistEntry() {};
        Song* key1;
        Song* key2;
        float dist;
        std::vector<Song *> candidates;
};

class Playlist
{
    public:
        Playlist(SongSet *songs, std::vector<Song *> keys, int add_tracks, float (*weights)[NUMBER_OF_AGGREGATE_STATS], int comparison_function);
        void print();
    private:
        Playlist() {};
        int add_tracks;
        std::vector<Song *> keys;
        std::vector<std::vector<PlaylistEntry *> > entries;
};

#endif
