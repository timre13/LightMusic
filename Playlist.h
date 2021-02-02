/*
BSD 2-Clause License

Copyright (c) 2021, timre13
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <vector>
#include <string>
#include "Music.h"

/*
 * This class represents a playlist containing tracks.
 */
class Playlist final
{
private:
    // Name of the output audio device
    std::string m_audioDevName;
    // We don't store the file contents, only the
    // filenames and open them on-the-fly
    std::vector<std::string> m_filePaths;
    // Index of currently played music in the playlist
    size_t m_currentTrackIndex{};
    // Currently played music
    Music *m_currentTrack{new Music};
    // If the playlist changed since last time
    bool m_isPlaylistChanged{true};

public:
    Playlist(const std::string &audioDevName);
    Playlist(const Playlist&) = delete;
    Playlist(Playlist&&) = delete;
    Playlist& operator=(const Playlist&) = delete;
    Playlist& operator=(Playlist&&) = delete;

    inline void addNewTrack(const std::string &filePath)
    {
        m_filePaths.push_back(filePath);
        m_isPlaylistChanged = true;
    }

    inline void removeTrack(size_t index)
    {
        m_filePaths.erase(m_filePaths.begin() + index);
        m_isPlaylistChanged = true;
    }

    inline size_t getNumOfTracks() const { return m_filePaths.size(); }
    inline int getCurrentTrackIndex() const { return m_currentTrackIndex; }
    inline std::string getTrackFilepathAt(size_t index) const
    {
        return m_filePaths[index];
    }
    inline std::string getCurrentTrackName() const
    {
        return m_filePaths.size() == 0 ? "" : m_filePaths[m_currentTrackIndex];
    }
    inline bool isPlaying() const
    {
        return m_currentTrack->getState() == Music::STATE_PLAYING;
    }
    bool hasEnded() const { return m_currentTrackIndex >= m_filePaths.size(); }

    Music* getCurrentTrack() { return m_currentTrack; }

    void openTrackAtIndex(size_t index);
    void startPlaying();

    void tickCurrentTrack();

    void unpauseCurrentTrack();
    void pauseCurrentTrack();

    void jumpToPrevTrack();
    void jumpToNextTrack();
    void reloadCurrentTrack();

    /*
     * Return whether the playlist has changed since the last time this
     * function was called.
     * If only the current index changed, it is not considered to be a change.
     */
    inline bool isPlaylistChangedSinceLastTime()
    {
        bool origVal{m_isPlaylistChanged};
        m_isPlaylistChanged = false;
        return origVal;
    }

    ~Playlist();
};
