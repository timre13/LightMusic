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
#include "config.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <FL/Enumerations.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>

MainWindow::MainWindow(int w, int h, const char *title, Playlist *playlistPtr)
    : Fl_Double_Window(w, h, title), m_playlistPtr{playlistPtr}
{
    color(FL_BLACK);
    begin();

   //-------------------------- Track info widgets --------------------------- 

    m_trackInfoBuffer = new Fl_Text_Buffer{};
    m_trackInfoW = new Fl_Text_Display{0, 0, 300, 320};
    m_trackInfoW->textsize(Fl_Fontsize{12});
    m_trackInfoW->textfont(FL_SCREEN);
    m_trackInfoW->textcolor(TEXT_COLOR);
    m_trackInfoW->color(BACKGROUND_COLOR);
    m_trackInfoW->buffer(m_trackInfoBuffer);
    m_trackInfoW->set_output();
    m_trackInfoW->end();

    m_playlistW = new Fl_Select_Browser{
            m_trackInfoW->w(), 0, w-m_trackInfoW->w(), m_trackInfoW->h()};
    m_playlistW->end();
    m_playlistW->textcolor(TEXT_COLOR);
    m_playlistW->color(BACKGROUND_COLOR);
    m_playlistW->selection_color(FL_GRAY);
    for (int i{}; i < m_playlistW->children(); ++i)
    {
        m_playlistW->array()[i]->color(BACKGROUND_COLOR);
        m_playlistW->array()[i]->color2(BUTTON_COLOR);
    }
    m_playlistW->callback(&s_playlistWidget_cb, this);

    //-------------------------------------------------------------------------

    //----------------------- Playlist control buttons ------------------------

    m_playlistBtnGrp = new Fl_Group{
            m_playlistW->x(), m_playlistW->h(), m_playlistW->w(), 20};

    m_addToPlaylistBtn = new Fl_Button{
            m_playlistBtnGrp->x(), m_playlistBtnGrp->y(), 20, 20};
    m_addToPlaylistBtn->copy_label("@+");
    m_addToPlaylistBtn->copy_tooltip("Add track to playlist...");
    m_addToPlaylistBtn->labelcolor(FL_GREEN);
    m_addToPlaylistBtn->color(BUTTON_COLOR);
    m_addToPlaylistBtn->callback(&s_addToPlaylistBtn_cb, this);

    m_removeFromPlaylistBtn = new Fl_Button{
            m_playlistBtnGrp->x()+20, m_playlistBtnGrp->y(), 20, 20};
    m_removeFromPlaylistBtn->copy_label("X");
    m_removeFromPlaylistBtn->copy_tooltip("Remove playing track from playlist");
    m_removeFromPlaylistBtn->labelcolor(FL_RED);
    m_removeFromPlaylistBtn->color(BUTTON_COLOR);
    m_removeFromPlaylistBtn->callback(&s_removeFromPlaylistBtn_cb, this);

    m_clearPlaylistBtn = new Fl_Button{
            m_playlistBtnGrp->x()+40, m_playlistBtnGrp->y(), 20, 20};
    m_clearPlaylistBtn->copy_label("[C]");
    m_clearPlaylistBtn->copy_tooltip("Clear playlist");
    m_clearPlaylistBtn->color(FL_RED);
    m_clearPlaylistBtn->labelcolor(FL_WHITE);
    m_clearPlaylistBtn->box(FL_FLAT_BOX);
    m_clearPlaylistBtn->callback(&s_clearPlaylistBtn_cb, this);

    m_shufflePlaylistBtn = new Fl_Button{
            m_playlistBtnGrp->x()+60, m_playlistBtnGrp->y(), 20, 20};
    m_shufflePlaylistBtn->copy_label("@refresh");
    m_shufflePlaylistBtn->copy_tooltip("Shuffle playlist");
    m_shufflePlaylistBtn->labelcolor(fl_rgb_color(100, 150, 255));
    m_shufflePlaylistBtn->color(BUTTON_COLOR);
    m_shufflePlaylistBtn->callback(&s_shufflePlaylistBtn_cb, this);

    m_playlistBtnGrp->end();

    //-------------------------------------------------------------------------
    
    //----------------------- Play control buttons ----------------------------

    m_ctrlBtnGrp = new Fl_Group{
            0, m_trackInfoW->h()+m_playlistBtnGrp->h(), 180, 60};

    m_prevTrackBtn  = new Fl_Button{0, m_ctrlBtnGrp->y()+10, 40, 40, "@<<"};
    m_prevTrackBtn->callback(s_prevTrackButton_cb, this);
    m_prevTrackBtn->labelcolor(FL_GREEN);
    m_prevTrackBtn->color(BUTTON_COLOR);
    m_prevTrackBtn->box(FL_ROUND_UP_BOX);
    m_prevTrackBtn->copy_tooltip("Jump to previous track");

    m_playPauseBtn = new Fl_Button{
            m_prevTrackBtn->x()+m_prevTrackBtn->w(), m_ctrlBtnGrp->y(), 60, 60, "@||"};
    m_playPauseBtn->callback(s_playPauseButton_cb, this);
    m_playPauseBtn->box(FL_ROUND_UP_BOX);
    m_playPauseBtn->labelcolor(FL_GREEN);
    m_playPauseBtn->color(BUTTON_COLOR);
    setPlayPauseButtonToPause();

    m_nextTrackBtn  = new Fl_Button{
            m_playPauseBtn->x()+m_playPauseBtn->w(), m_ctrlBtnGrp->y()+10,
            40, 40, "@>>"};
    m_nextTrackBtn->callback(s_nextTrackButton_cb, this);
    m_nextTrackBtn->labelcolor(FL_GREEN);
    m_nextTrackBtn->color(BUTTON_COLOR);
    m_nextTrackBtn->box(FL_ROUND_UP_BOX);
    m_nextTrackBtn->copy_tooltip("Jump to next track");

    m_stopBtn       = new Fl_Button{
            m_nextTrackBtn->x()+m_nextTrackBtn->w(), m_ctrlBtnGrp->y()+10,
            40, 40, "@square"};
    m_stopBtn->callback(s_stopButton_cb, this);
    m_stopBtn->labelcolor(FL_GREEN);
    m_stopBtn->color(BUTTON_COLOR);
    m_stopBtn->box(FL_ROUND_UP_BOX);
    m_stopBtn->copy_tooltip("Stop");

    m_ctrlBtnGrp->end();

    //-------------------------------------------------------------------------

    //------------------------ Progress display widgets -----------------------

    m_timeLabelBuffer = new Fl_Text_Buffer{16};
    m_timeLabel = new Fl_Text_Display{
            m_ctrlBtnGrp->w()+10, m_trackInfoW->h()+m_playlistBtnGrp->h()+5,
            w-m_ctrlBtnGrp->w()-20, 20};
    m_timeLabel->buffer(m_timeLabelBuffer);
    m_timeLabel->set_output();
    m_timeLabel->box(FL_NO_BOX);
    m_timeLabel->color(BACKGROUND_COLOR);
    m_timeLabel->textcolor(FL_GREEN);
    m_timeLabelBuffer->text("00:00:00/00:00:00");

    m_progressBar = new Fl_Hor_Nice_Slider{
            m_ctrlBtnGrp->w()+10, m_trackInfoW->h()+m_playlistBtnGrp->h()+35,
            w-m_ctrlBtnGrp->w()-20, 20};
    m_progressBar->color(BUTTON_COLOR);
    m_progressBar->color2(FL_GREEN);
    m_progressBar->minimum(0.0);
    m_progressBar->callback(&s_progressBar_cb, this);

    //-------------------------------------------------------------------------

    end();

    m_playPauseBtn->take_focus();

    Fl::add_idle(s_tickMusic, this);
    Fl::add_timeout(0, s_updateGui, this);

    auto icon{std::make_unique<Fl_PNG_Image>("img/icon.png")};
    if (icon->fail())
        std::cerr << "Failed to load icon" << '\n';
    else
        this->icon(icon.get());

    resizable(m_playlistW);
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
    if (m_playlistPtr->isPlaylistChangedSinceLastTime())
    {
        m_playlistW->clear();

        for (size_t i{}; i < m_playlistPtr->getNumOfTracks(); ++i)
        {
            const std::string trackFilepath{
                m_playlistPtr->getTrackFilepathAt(i)};
            const std::string trackFilename{
                trackFilepath.substr(trackFilepath.find_last_of('/')+1)};

            m_playlistW->add(trackFilename.c_str());
        }
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

    // Update the stop button
    if (m_stopBtn->active() && !m_playlistPtr->isPlaying())
        m_stopBtn->deactivate();
    else if (!m_stopBtn->active() && m_playlistPtr->isPlaying())
        m_stopBtn->activate();

    Fl::repeat_timeout(0.5, s_updateGui, this);
}

//--------------------- Play control button callbacks -------------------------

void MainWindow::playPauseButton_cb()
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
    updateGui();
}

void MainWindow::prevTrackButton_cb()
{
    m_playlistPtr->jumpToPrevTrack();
    setPlayPauseButtonToPause();
    updateGui();
}

void MainWindow::nextTrackButton_cb()
{
    m_playlistPtr->jumpToNextTrack();
    setPlayPauseButtonToPause();
    updateGui();
}

void MainWindow::stopButton_cb()
{
    m_playlistPtr->getCurrentTrack()->seekToS(0);
    m_playlistPtr->tickCurrentTrack();
    m_playlistPtr->pauseCurrentTrack();
    setPlayPauseButtonToPlay();
    updateGui();
}

//-----------------------------------------------------------------------------

void MainWindow::playlistWidget_cb()
{
    int selectedLine{m_playlistW->value()};
    if (selectedLine == 0) // If no line selected, don't change track
        return;

    m_playlistPtr->openTrackAtIndex(selectedLine-1);

    // Hack: We first select line 1, so the next selection triggers the blue selection
    m_playlistW->select(1);
    // Select the track that could be opened
    m_playlistW->select(m_playlistPtr->getCurrentTrackIndex() + 1);

    // When clicking on a track, we start playing,
    // so update the play/pause button
    setPlayPauseButtonToPause();
}

void MainWindow::progressBar_cb()
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

//--------------------- Playlist control button callbacks ---------------------

void MainWindow::addToPlaylistBtn_cb()
{
    auto filepath = fl_file_chooser("Select a file...", "", "*");
    if (filepath)
    {
        m_playlistPtr->addNewTrack(filepath);

        // Open the newly added track
        m_playlistPtr->openTrackAtIndex(m_playlistPtr->getNumOfTracks() - 1);

        updateGui();
    }
}

void MainWindow::removeFromPlaylistBtn_cb()
{
    m_playlistPtr->removeTrack(m_playlistPtr->getCurrentTrackIndex());

    updateGui();
}

void MainWindow::clearPlaylistBtn_cb()
{
    m_playlistPtr->removeAllTracks();

    updateGui();
}

void MainWindow::shufflePlaylistBtn_cb()
{
    m_playlistPtr->shuffle();

    updateGui();
}

//-----------------------------------------------------------------------------

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

