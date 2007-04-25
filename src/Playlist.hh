// $Id$

#ifndef PLAYLIST_HH
#define PLAYLIST_HH


bool playlist_song_cmp(Song* s1, Song* s2);

class PlaylistEntry
{
    public:
        PlaylistEntry(SongSet *candidiates, Song *key1, Song* key2, float dist);
        PlaylistEntry(Song *key);
        void removeArtist(std::string artist);
        void removeTrack(std::string filename);
        Song *getBestMatch();
        float getScore();
        
        PlaylistEntry* neighbour1;
        PlaylistEntry* neighbour2;
    private:
        PlaylistEntry() {};
        Song* key;
        bool isKey;
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
        std::vector<PlaylistEntry *> keys;
        std::vector<std::vector<PlaylistEntry *> > entries;
};

#endif
