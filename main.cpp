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

#include <iostream>
#include <memory>
extern "C"
{
#include <libavdevice/avdevice.h>
}
#include <FL/Fl.H>
#include "Playlist.h"
#include "MainWindow.h"
#include "version.h"

#define AUDIO_DEV_NAME "alsa" // TODO: Windows compatibility

int main(int argc, char **argv)
{
    std::cout << "LightMusic music player version " VERSION_STR << '\n';

    // Init audio I/O
    avdevice_register_all();

    auto playlist{std::make_unique<Playlist>(AUDIO_DEV_NAME)};

    if (argc <= 1) // When running as a test
    {
        std::cout << "Using test music files" << '\n';
        playlist->addNewTrack("nonpublic-test-music/csp_short.wav");
        playlist->addNewTrack("nonpublic-test-music/sd.opus");
        playlist->addNewTrack("nonpublic-test-music/csp.wav");
        playlist->addNewTrack("nonpublic-test-music/csp.mp3");
        playlist->addNewTrack("nonpublic-test-music/csp.ogg");
        playlist->addNewTrack("nonpublic-test-music/des.mp3");
        playlist->addNewTrack("nonpublic-test-music/b.mkv");
    }
    else
    {
        for (int i{1}; i < argc; ++i)
            playlist->addNewTrack(argv[i]);
    }

    playlist->startPlaying();

    auto mainWindow{
        std::make_unique<MainWindow>(800, 400, "LightMusic", playlist.get())};
    mainWindow->show();

    return Fl::run();
}
