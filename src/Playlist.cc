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
    this->dist = dist;
    FeatureGroup entry_desired(key1->song_features, key2->song_features, dist);
    playlist_desired = &entry_desired;
    std::sort(this->candidates.begin(), this->candidates.end(), playlist_song_cmp);
    this->isKey = false;
}

PlaylistEntry::PlaylistEntry(Song *key)
{
    this->key = key;
    this->isKey = true;
}

float PlaylistEntry::getScore()
{
    float neighbour_score = 0;
    neighbour_score += neighbour1->getBestMatch()->compare(getBestMatch(), playlist_weights, playlist_comparison_function);
    neighbour_score += neighbour2->getBestMatch()->compare(getBestMatch(), playlist_weights, playlist_comparison_function);
    return (std::isnormal(neighbour_score) ? neighbour_score : 9999.0f);
}

Song* PlaylistEntry::getBestMatch()
{
    if (isKey) {
        return key;
    } else {
        return candidates.at(0);
    }
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

void PlaylistEntry::removeTrack(std::string filename)
{
    for (unsigned int i = 0; i < candidates.size(); i++) {
        if (candidates.at(i)->filename == filename) {
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
    
    // create playlistentries
    for (unsigned int key = 0; key < keys.size() - 1; key++) {
        this->keys.push_back(new PlaylistEntry(keys.at(key)));
        Song *key1 = keys.at(key);
        Song *key2 = keys.at(key + 1);
        entries.push_back(std::vector<PlaylistEntry *>());
        for (int entry = 0; entry < add_tracks; entry++) {
            entries.back().push_back(new PlaylistEntry(songs, key1, key2, ((float)(entry + 1) / (add_tracks + 1))));
        }
    }
    this->keys.push_back(new PlaylistEntry(keys.back()));
    
    // set neighbours for continuity comparison
    for (unsigned int key = 0; key < keys.size() - 1; key++) {
        entries[key][0]->neighbour1 = this->keys[key];
        entries[key][0]->neighbour2 = entries[key][1];
        for (unsigned int entry = 1; entry < entries.at(key).size() - 1; entry++) {
            entries[key][entry]->neighbour1 = entries[key][entry - 1];
            entries[key][entry]->neighbour2 = entries[key][entry + 1];
        }
        entries[key][entries.at(key).size() - 1]->neighbour1 = entries[key][entries.at(key).size() - 2];
        entries[key][entries.at(key).size() - 1]->neighbour2 = this->keys[key + 1];
    }
    
    for (unsigned int key = 0; key < keys.size() - 1; key++) {
        std::string key_artist = this->keys.at(key)->getBestMatch()->get_artist();
        for (unsigned int entry = 0; entry < entries.at(key).size(); entry++) {
            entries.at(key).at(entry)->removeArtist(key_artist);
        }
        if (key > 0) {
            for (unsigned int entry = 0; entry < entries.at(key - 1).size(); entry++) {
                entries.at(key - 1).at(entry)->removeArtist(key_artist);
            }
        }
    }
    
    for (unsigned int entry = 0; entry < entries.back().size(); entry++) {
        entries.back().at(entry)->removeArtist(keys.back()->get_artist());
    }
    
    // remove tracks by the same artist between two keys
    int playlist_bad = 1;
    while (playlist_bad != 0) {
        playlist_bad = 0;
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
        
        // remove identical tracks from playlist
        int num_bad = 1;
        while (num_bad != 0) {
            num_bad = 0;
            std::map<std::string, std::vector<PlaylistEntry *> > tracks;
            for (unsigned int key = 0; key < keys.size() - 1; key++) {
                for (unsigned int entry = 0; entry < entries[key].size(); entry++) {
                    //std::cout << key << ";" << ";" << entry << ";" << entries[key][entry]->getBestMatch()->get_artist() << ";" << entries[key][entry]->getBestMatch()->get_album() << ";" << entries[key][entry]->getBestMatch()->filename << std::endl;
                    std::string new_filename = entries[key][entry]->getBestMatch()->filename;
                    std::map<std::string, std::vector<PlaylistEntry *> >::iterator other_tracks = tracks.find(new_filename);
                    if (other_tracks == tracks.end()) {
                        std::vector<PlaylistEntry *> new_vector;
                        new_vector.push_back(entries[key][entry]);
                        tracks[new_filename] = new_vector;
                    } else {
                        other_tracks->second.push_back(entries[key][entry]);
                    }
                }
            }
            
            for(
                std::map<std::string, std::vector<PlaylistEntry *> >::iterator iter = tracks.begin();
                iter != tracks.end();
                iter++
            ) {
                std::string filename = iter->first;
                std::vector<PlaylistEntry *> bad_entries = iter->second;
                
                if (bad_entries.size() == 1) { continue; } else { num_bad++; }
                
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
                    bad_entries[i]->removeTrack(filename);
                    playlist_bad = 1;
                }
            }
        }
    }
    for (unsigned int key = 0; key < keys.size() - 1; key++) {
        for (unsigned int entry = 0; entry < entries[key].size(); entry++) {
            std::cout << key << ";" << ";" << entry << ";" << entries[key][entry]->getBestMatch()->get_artist() << ";" << entries[key][entry]->getBestMatch()->get_album() << ";" << entries[key][entry]->getBestMatch()->filename << std::endl;
        }
    }
}

void Playlist::print()
{
    for (unsigned int key = 0; key < keys.size() - 1; key++) {
        std::cout << keys.at(key)->getBestMatch()->filename.substr(0, keys.at(key)->getBestMatch()->filename.length() - 4) << std::endl;
        for (unsigned int entry = 0; entry < entries[key].size(); entry++) {
            std::string filename = entries[key][entry]->getBestMatch()->filename;
            std::cout << filename.substr(0, filename.length() - 4) << std::endl;
        }
    }
    std::cout << keys.back()->getBestMatch()->filename.substr(0, keys.back()->getBestMatch()->filename.length() - 4) << std::endl;
}

