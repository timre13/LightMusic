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

#include "MainWindow.h"
#include "AboutWindow.h"
#include "version.h"
#include "sys-specific.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_ask.H>

MainWindow::MainWindow(int w, int h, const char *title, Playlist *playlistPtr)
    : Fl_Double_Window(w, h, title), m_playlistPtr{playlistPtr}
{
    if (!Fl::scheme("plastic"))
        std::cerr << "Failed to set scheme" << '\n';

    begin();

    m_trackInfoBuffer = new Fl_Text_Buffer{};
    m_trackInfoW = new Fl_Text_Display{0, 0, 300, 340};
    m_trackInfoW->textsize(Fl_Fontsize{12});
    m_trackInfoW->textfont(FL_SCREEN);
    m_trackInfoW->buffer(m_trackInfoBuffer);
    m_trackInfoW->set_output();
    m_trackInfoW->end();

    m_playlistW = new Fl_Select_Browser{m_trackInfoW->w(), 0, w-m_trackInfoW->w(), m_trackInfoW->h()};
    m_playlistW->end();
    m_playlistW->callback(&s_playlistWidgetCallback, this);

    m_ctrlBtnGrp = new Fl_Group{0, m_trackInfoW->h(), 180, h-m_trackInfoW->h()};
    m_ctrlBtnGrp->end();

    m_timeLabelBuffer = new Fl_Text_Buffer{16};
    m_timeLabel = new Fl_Text_Display{m_ctrlBtnGrp->w()+10, m_trackInfoW->h()+5, w-m_ctrlBtnGrp->w()-20, 20};
    m_timeLabel->buffer(m_timeLabelBuffer);
    m_timeLabel->set_output();
    m_timeLabel->box(FL_NO_BOX);
    m_timeLabel->color(FL_BACKGROUND_COLOR);
    m_timeLabelBuffer->text("00:00:00/00:00:00");

    m_progressBar = new Fl_Hor_Nice_Slider{m_ctrlBtnGrp->w()+10, m_trackInfoW->h()+35, w-m_ctrlBtnGrp->w()-20, 20};
    m_progressBar->minimum(0.0);
    m_progressBar->callback(&s_progressBarCallback, this);

    end();

    m_ctrlBtnGrp->begin();
    m_prevTrackBtn  = new Fl_Button{0, m_ctrlBtnGrp->y()+10, 40, 40, "@<<"};
    m_prevTrackBtn->callback(s_onPrevTrackButtonPressed, this);
    m_prevTrackBtn->labelcolor(FL_YELLOW);
    m_prevTrackBtn->box(FL_ROUND_UP_BOX);
    m_prevTrackBtn->copy_tooltip("Jump to previous track");
    m_playPauseBtn = new Fl_Button{m_prevTrackBtn->x()+m_prevTrackBtn->w(), m_ctrlBtnGrp->y(), 60, 60, "@||"};
    m_playPauseBtn->callback(s_onPlayPauseButtonPressed, this);
    m_playPauseBtn->box(FL_ROUND_UP_BOX);
    setPlayPauseButtonToPause();
    m_nextTrackBtn  = new Fl_Button{m_playPauseBtn->x()+m_playPauseBtn->w(), m_ctrlBtnGrp->y()+10, 40, 40, "@>>"};
    m_nextTrackBtn->callback(s_onNextTrackButtonPressed, this);
    m_nextTrackBtn->labelcolor(FL_YELLOW);
    m_nextTrackBtn->box(FL_ROUND_UP_BOX);
    m_nextTrackBtn->copy_tooltip("Jump to next track");
    m_stopBtn       = new Fl_Button{m_nextTrackBtn->x()+m_nextTrackBtn->w(), m_ctrlBtnGrp->y()+10, 40, 40, "@square"};
    m_stopBtn->labelcolor(FL_RED);
    m_stopBtn->box(FL_ROUND_UP_BOX);
    m_stopBtn->copy_tooltip("Stop");
    m_ctrlBtnGrp->end();

    Fl::add_idle(s_tickMusic, this);
    Fl::add_timeout(0, s_updateGui, this);

    auto icon{std::make_unique<Fl_PNG_Image>("img/icon.png")};
    if (icon->fail())
        std::cerr << "Failed to load icon" << '\n';
    else
        this->icon(icon.get());
}

void MainWindow::tickMusic()
{
    if (!m_playlistPtr->hasEnded())
        m_playlistPtr->tickCurrentTrack();

    // If not playing, don't hog the CPU
    if (!m_playlistPtr->isPlaying())
        SysSpecific::wait(0.1);
}

static std::string timeToString(int64_t timeInSeconds)
{
    std::stringstream ss;
    ss << std::setfill('0') <<
        std::setw(2) << timeInSeconds / 60 / 60 << ':' <<
        std::setw(2) << timeInSeconds % (60 * 60) / 60 << ':' <<
        std::setw(2) << timeInSeconds % 60;

    return ss.str();
}

void MainWindow::updateGui()
{
    // FIXME: Breaks if track if replaced, not removed/added
    if (m_playlistPtr->getNumOfTracks() != (size_t)m_playlistW->size())
    {
        m_playlistW->clear();
        for (size_t i{}; i < m_playlistPtr->getNumOfTracks(); ++i)
            m_playlistW->add(m_playlistPtr->getTrackFilepathAt(i).substr(m_playlistPtr->getTrackFilepathAt(i).find_last_of('/')+1).c_str());
        m_playlistW->select(m_playlistPtr->getCurrentTrackIndex()+1);
    }
    if (!m_playlistW->selected(m_playlistPtr->getCurrentTrackIndex()+1))
        m_playlistW->select(m_playlistPtr->getCurrentTrackIndex()+1);

    auto currentTrack{m_playlistPtr->getCurrentTrack()};

    m_timeLabelBuffer->text((
             timeToString(currentTrack->getCurrentTimestampS()) +
             "/" +
             timeToString(currentTrack->getDurationS())).c_str());

    m_progressBar->maximum(currentTrack->getDurationS());
    m_progressBar->value(currentTrack->getCurrentTimestampS());

    std::string trackInfoBuffer;
    trackInfoBuffer += currentTrack->getFileInfo();
    trackInfoBuffer += "Aud. stream:\n";
    trackInfoBuffer += currentTrack->getAudioStreamInfo();
        m_trackInfoBuffer->text(trackInfoBuffer.c_str());

    Fl::repeat_timeout(0.5, s_updateGui, this);
}

void MainWindow::onPlayPauseButtonPressed()
{
    if (m_playlistPtr->isPlaying())
    {
        m_playlistPtr->pauseCurrentTrack();

        setPlayPauseButtonToPlay();
    }
    else
    {
        m_playlistPtr->unpauseCurrentTrack();
        setPlayPauseButtonToPause();
    }
}

void MainWindow::onPrevTrackButtonPressed()
{
    m_playlistPtr->jumpToPrevTrack();
    setPlayPauseButtonToPause();
}

void MainWindow::onNextTrackButtonPressed()
{
    m_playlistPtr->jumpToNextTrack();
    setPlayPauseButtonToPause();
}

void MainWindow::playlistWidgetCallback()
{
    int selectedLine{m_playlistW->value()};
    if (selectedLine == 0) // If no line selected, don't change track
        return;

    m_playlistPtr->openTrackAtIndex(selectedLine-1);
    m_playlistW->select(selectedLine);

    // When clicking on a track, we start playing,
    // so update the play/pause button
    setPlayPauseButtonToPause();
}

void MainWindow::progressBarCallback()
{
    m_playlistPtr->getCurrentTrack()->seekToS(m_progressBar->value());

    // We tick the track to update the internal state, so
    // updateGui() gets the actual timestamp, not the old one
    if (!m_playlistPtr->isPlaying())
    {
        // Cannot tick if paused, so unpause
        m_playlistPtr->unpauseCurrentTrack();
        m_playlistPtr->tickCurrentTrack();
        // Pause it, it was the original state
        m_playlistPtr->pauseCurrentTrack();
    }
    else
    {
        m_playlistPtr->tickCurrentTrack();
    }

    // Update the `remaining time widget` and the progressbar
    updateGui();
}

int MainWindow::handle(int event)
{
    switch (Fl::event_key())
    {
    case FL_Escape:
        // Disable closing with ESC
        return 1;
    case FL_F + 1:
        // Show the about dialog when F1 is pressed
        showAboutDialog();
        return 1;
    }

    return this->Fl_Double_Window::handle(event);
}

void MainWindow::showAboutDialog()
{
    if (!m_isAboutWindowShown)
    {
        m_isAboutWindowShown = true;

        auto dialog{std::make_unique<AboutWindow>()};
        dialog->set_modal();
        dialog->show();
        while (dialog->shown())
            Fl::wait();

        m_isAboutWindowShown = false;
    }
}

MainWindow::~MainWindow()
{
}

