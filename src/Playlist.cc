// $Id$

#include <string>
#include <vector>

#include <cmath>
#include <map>
#include <cfloat>

#include "common.hh"
#include "Song.hh"
#include "SongSet.hh"
#include "Playlist.hh"


FeatureGroup *playlist_desired;
float (*playlist_weights)[NUMBER_OF_AGGREGATE_STATS];
int playlist_comparison_function;

PlaylistEntry::PlaylistEntry(SongSet *candidates, Song *key1, Song* key2, float dist)
{
    for (unsigned int i = 0; i < candidates->size(); i++) {
        this->candidates.push_back(candidates->at(i));
    }
    this->key1 = key1;
    this->key2 = key2;
    this->dist = dist;
    FeatureGroup entry_desired(key1->song_features, key2->song_features, dist);
    playlist_desired = &entry_desired;
    std::sort(this->candidates.begin(), this->candidates.end(), playlist_song_cmp);
}

float PlaylistEntry::getScore()
{
    float score = (key1->compare(getBestMatch(), playlist_weights, playlist_comparison_function) * dist) +
                  (key2->compare(getBestMatch(), playlist_weights, playlist_comparison_function) * (1 - dist));
    return (std::isnormal(score) ? score : 9999.0f);
}

Song* PlaylistEntry::getBestMatch()
{
    return candidates.at(0);
}

void PlaylistEntry::removeArtist(std::string artist)
{
    for (unsigned int i = 0; i < candidates.size(); i++) {
        if (candidates.at(i)->get_artist() == artist) {
            candidates.erase(candidates.begin() + i);
            i--;
        }
    }
}

bool playlist_song_cmp(Song* s1, Song* s2)
{
    return playlist_desired->compare(s1->song_features, playlist_weights) < playlist_desired->compare(s2->song_features, playlist_weights);
}

Playlist::Playlist(SongSet *songs, std::vector<Song *> keys, int add_tracks, float (*weights)[NUMBER_OF_AGGREGATE_STATS], int comparison_function)
{
    playlist_weights = weights;
    playlist_comparison_function = comparison_function;
    this->add_tracks = add_tracks;
    
    for (unsigned int key = 0; key < keys.size() - 1; key++) {
        this->keys.push_back(keys.at(key));
        Song *key1 = keys.at(key);
        Song *key2 = keys.at(key + 1);
        entries.push_back(std::vector<PlaylistEntry *>());
        for (int entry = 0; entry < add_tracks; entry++) {
            entries.back().push_back(new PlaylistEntry(songs, key1, key2, ((float)(entry + 1) / (add_tracks + 1))));
        }
    }
    
    for (unsigned int key = 0; key < keys.size() - 1; key++) {
        std::string key_artist = this->keys.at(key)->get_artist();
        for (unsigned int entry = 0; entry < entries.at(key).size(); entry++) {
            entries.at(key).at(entry)->removeArtist(key_artist);
        }
        if (key > 0) {
            for (unsigned int entry = 0; entry < entries.at(key - 1).size(); entry++) {
                entries.at(key - 1).at(entry)->removeArtist(key_artist);
            }
        }
    }
    
    this->keys.push_back(keys.back());
    for (unsigned int entry = 0; entry < entries.back().size(); entry++) {
        entries.back().at(entry)->removeArtist(keys.back()->get_artist());
    }
    
    for (unsigned int key = 0; key < keys.size() - 1; key++) {
        int num_bad = 1;
        while (num_bad != 0) {
            num_bad = 0;
            std::map<std::string, std::vector<PlaylistEntry *> > artist_map;
            
            for (unsigned int entry = 0; entry < entries[key].size(); entry++) {
                std::string new_artist = entries[key][entry]->getBestMatch()->get_artist();
                std::map<std::string, std::vector<PlaylistEntry *> >::iterator other_tracks = artist_map.find(new_artist);
                if (other_tracks == artist_map.end()) {
                    std::vector<PlaylistEntry *> new_vector;
                    new_vector.push_back(entries[key][entry]);
                    artist_map[new_artist] = new_vector;
                } else {
                    other_tracks->second.push_back(entries[key][entry]);
                    num_bad++;
                }
            }
            
            for(
                std::map<std::string, std::vector<PlaylistEntry *> >::iterator iter = artist_map.begin();
                iter != artist_map.end();
                iter++
            ) {
                std::string artist = iter->first;
                std::vector<PlaylistEntry *> bad_entries = iter->second;
                
                int best_pos = 0;
                float best_score = bad_entries[0]->getScore();
                for (unsigned int i = 1; i < bad_entries.size(); i++) {
                    if (bad_entries[i]->getScore() < best_score) {
                        best_pos = i;
                        best_score = bad_entries[i]->getScore();
                    }
                }
                bad_entries.erase(bad_entries.begin() + best_pos);
                for (unsigned int i = 0; i < bad_entries.size(); i++) {
                    bad_entries[i]->removeArtist(artist);
                }
            }
        }
    }
}

void Playlist::print()
{
    for (unsigned int key = 0; key < keys.size() - 1; key++) {
        std::cout << keys.at(key)->filename << std::endl;
        for (unsigned int entry = 0; entry < entries[key].size(); entry++) {
            std::cout << entries[key][entry]->getBestMatch()->filename << std::endl;
        }
    }
    std::cout << keys.back()->filename << std::endl;
}

