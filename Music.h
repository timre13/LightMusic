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

#include <string>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
}

/*
 * Represents a music file in the memory with all of its states like format,
 * codec, output device, output stream, output format, output context.
 */
class Music final
{
public:
    enum State
    {
        // No file open
        STATE_UNINITIALIZED,
        // Paused, `tick()` does nothing
        STATE_PAUSED,
        // Playing, `tick()` plays the file
        STATE_PLAYING,
        // End of file, `tick()` does nothing
        // EOF can be detected from outside by checking this
        STATE_END,
        // Set by internal function if something bad happens
        STATE_ERROR,
    };

    enum OpenError
    {
        // Successfully opened file
        OPENERROR_OK, // Always 0!
        // Failed to allocate something, can be caused by memory error,
        // OS error or bad parameter to allocator function.
        OPENERROR_ALLOC,
        // Failed to open or get properties of input file
        OPENERROR_FILE,
        // Failed to open, initialize or get properties of output device
        OPENERROR_OUTPUT,
        // Other error
        OPENERROR_OTHER,
    };

private:
    State             m_state{};
    AVFormatContext   *m_formatContext{};
    AVCodecParameters *m_codecParams{};
    AVCodec           *m_codec{};
    AVCodecContext    *m_codecContext{};
    AVOutputFormat    *m_outputFormat{};
    AVFormatContext   *m_outputFormatContext{};
    AVStream          *m_outputStream{};
    SwrContext        *m_resampleContext{};
    AVPacket          *m_currentPacket{};
    int               m_audioStreamI{};

private:
    /*
     * Open an audio device with the passed name.
     * Should only be called by `open()`.
     *
     * Returns 0 if succeeded, nonzero otherwise.
     */
    int openAudioDevice(const std::string &audioDevName);

    /*
     * Initializes the resample context using the device
     * properties and the input properties.
     * Should only be called by `open()`.
     *
     * Returns 0 if succeeded, nonzero otherwise.
     */
    int initResampleContext();

public:
    Music();
    Music(const Music&) = delete;
    Music(Music&&) = delete;
    Music& operator=(const Music&) = delete;
    Music& operator=(Music&&) = delete;

    /*
     * Open a file with the specified path,
     * find a valid audio stream, find a codec,
     * print some info, call `openAudioDevice()`
     * and call `initResampleContext()`.
     *
     * Returns an `OpenError` value.
     */
    OpenError open(
            const std::string &filePath,
            const std::string &audioDevName);

    /*
     * Check if the object is in a usable state, read a frame from the input
     * file, decode it, resample it and write it to the output device.
     *
     * Should be called repeatedly until the music
     * ends (`hasEnded()` returns true).
     */
    void tick();

    /*
     * Check if the object is in a usable state.
     * If yes, set `m_state` to `STATE_PLAYING`.
     */
    void unPause();
    /*
     * Check if the object is in a usable state.
     * If yes, set `m_state` to `STATE_PAUSED`.
     */
    void pause();

    /*
     * Close the input file, the output device, free stuff and call `reset()`.
     */
    void closeAndReset();
    /*
     * Set the internal state to the default (everything is zero).
     */
    void reset();

    /*
     * Seek to the specified second.
     */
    void seekToS(double timestamp);

    inline State getState() const { return m_state; }
    inline bool hasEnded() const { return m_state == STATE_END; }
    inline bool isInErrorState() const { return m_state == STATE_ERROR; }

    inline int64_t getDurationS() const { return m_formatContext ? m_formatContext->duration/AV_TIME_BASE : 0; }
    // FIXME: MP3 timestamps are weird
    inline int64_t getCurrentTimestampS() const { return m_currentPacket ? m_currentPacket->pts * av_q2d(m_outputStream->time_base) : 0; }
    std::string getFileInfo() const;
    std::string getAudioStreamInfo() const;

    /*
     * Close the file, the output device and free everything.
     */
    ~Music();
};
