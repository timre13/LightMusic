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

#include "Music.h"

#include <iostream>
#include <cassert>

Music::Music()
{
    reset();
}

void Music::reset()
{
    m_state = STATE_UNINITIALIZED;

    m_formatContext       = nullptr;
    m_codec               = nullptr;
    m_codecContext        = nullptr;
    m_outputFormat        = nullptr;
    m_outputFormatContext = nullptr;
    m_outputStream        = nullptr;
    m_resampleContext     = nullptr;
}

static void printFileInfo(const AVFormatContext *formatContext)
{
    // Print some info
    std::cout << "Filename/url: " << formatContext->url << '\n';
    std::cout << "Metadata:" << '\n';
    AVDictionaryEntry *metadataTag{nullptr};
    while ((metadataTag = av_dict_get(formatContext->metadata, "", metadataTag, AV_DICT_IGNORE_SUFFIX)))
        std::cout << "\t" << metadataTag->key << ": " << metadataTag->value << '\n';
    std::cout << "Format: " << formatContext->iformat->long_name << '\n';
    std::cout << "Duration: " << (long double)formatContext->duration / AV_TIME_BASE << "s" << '\n';
    std::cout << "Bit rate: " << formatContext->bit_rate << '\n';
}

int Music::openAudioDevice(const std::string &audioDevName)
{
    // Get the output format of the audio device
    m_outputFormat = av_guess_format(audioDevName.c_str(), nullptr, nullptr);
    if (!m_outputFormat)
    {
        std::cerr << "Failed to get format of output device" << '\n';
        return 1;
    }

    m_outputFormatContext = avformat_alloc_context();
    if (!m_outputFormatContext)
    {
        std::cerr << "Failed to create output format context" << '\n';
        return 1;
    }
    // Tell the format context which output device to use
    m_outputFormatContext->oformat = m_outputFormat;

    // Create a stream where the output will be written to
    m_outputStream = avformat_new_stream(m_outputFormatContext, nullptr);
    if (!m_outputStream)
    {
        std::cerr << "Failed to create output stream" << '\n';
        return 1;
    }
    // Configure the parameters of the output stream
    m_outputStream->codecpar->codec_id       = AV_CODEC_ID_PCM_S16LE;
    m_outputStream->codecpar->codec_type     = AVMEDIA_TYPE_AUDIO;
    m_outputStream->codecpar->format         = AV_SAMPLE_FMT_S16;
    m_outputStream->codecpar->bit_rate       = m_codecContext->bit_rate;
    m_outputStream->codecpar->sample_rate    = m_codecContext->sample_rate;
    m_outputStream->codecpar->channels       = m_codecContext->channels;
    m_outputStream->codecpar->channel_layout = AV_CH_LAYOUT_STEREO;

    // Tell the device the parameters
    if (avformat_write_header(m_outputFormatContext, nullptr) < 0)
    {
        std::cerr << "Failed to write stream header to output device" << '\n';
        return 1;
    }

    return 0;
}

int Music::initResampleContext()
{
    m_resampleContext =
            swr_alloc_set_opts(
                    nullptr,
                    m_outputStream->codecpar->channel_layout,         // Out channel layout
                    (AVSampleFormat)m_outputStream->codecpar->format, // Out sample format
                    m_outputStream->codecpar->sample_rate,            // Out sample rate
                    AV_CH_LAYOUT_STEREO,                              // In channel layout
                    (AVSampleFormat)m_codecContext->sample_fmt,       // In format
                    m_codecContext->sample_rate,                      // In sample rate
                    0,
                    nullptr);
    if (!m_resampleContext)
    {
        std::cerr << "Failed to create resample context" << '\n';
        return 1;
    }

    if (swr_init(m_resampleContext))
    {
        std::cerr << "Failed to init resample context" << '\n';
        return 1;
    }

    return 0;
}

Music::OpenError Music::open(
        const std::string &filePath,
        const std::string &audioDevName)
{
    assert(m_state == STATE_UNINITIALIZED);

    m_formatContext = avformat_alloc_context();
    if (!m_formatContext)
    {
        std::cerr << "Failed to allocate format context" << '\n';
        m_state = STATE_ERROR;
        return OPENERROR_ALLOC;
    }

    // Open the file as input to the format context
    if (avformat_open_input(
            &m_formatContext,
            filePath.c_str(),
            nullptr,
            nullptr))
    {
        std::cerr << "Failed to open file" << '\n';
        m_state = STATE_ERROR;
        return OPENERROR_FILE;
    }

    printFileInfo(m_formatContext);

    // Find the info of the streams, so we can use them
    if (avformat_find_stream_info(m_formatContext, nullptr))
    {
        std::cerr << "Failed to find stream info" << '\n';
        m_state = STATE_ERROR;
        return OPENERROR_OTHER;
    }
    std::cout << "Number of streams: " << m_formatContext->nb_streams << '\n';

    // Loop through the streams
    for (unsigned int i{}; i < m_formatContext->nb_streams; ++i)
    {
        auto codecParameters{m_formatContext->streams[i]->codecpar};
        // Find a codec for the stream
        m_codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!m_codec)
        {
            std::cerr << "Failed to find decoder for stream, skipping" << '\n';
            continue;
        }

        // Print info of the stream
        std::cout << "Stream #" << i << ":" << '\n';
        std::cout << "\tCodec name: " << m_codec->long_name << '\n';
        std::cout << "\tCodec ID: " << m_codec->id << '\n';
        std::cout << "\tBit rate: " << codecParameters->bit_rate << '\n';

        // If this is not an audio stream
        if (m_codec->type != AVMEDIA_TYPE_AUDIO)
        {
            std::cout << "\tNot an audio stream, skipping" << '\n';
            continue;
        }

        std::cout << "\tChannels: " << codecParameters->channels << '\n';
        std::cout << "\tSample rate: " << codecParameters->sample_rate << '\n';

        m_codecContext = avcodec_alloc_context3(m_codec);
        if (!m_codecContext)
        {
            std::cerr << "\tFailed to allocate codec context, skipping" << '\n';
            continue;
        }

        if (avcodec_parameters_to_context(m_codecContext, codecParameters))
        {
            std::cerr << "\tFailed to fill codex context, skipping" << '\n';
            avcodec_free_context(&m_codecContext);
            continue;
        }

        char sampleFormatStr[32];
        std::cout << "\tSample format: " << av_get_sample_fmt_string(
                sampleFormatStr,
                sizeof(sampleFormatStr),
                m_codecContext->sample_fmt) << '\n';

        if (avcodec_open2(m_codecContext, m_codec, nullptr))
        {
            std::cerr << "Failed to open codex context, skipping" << '\n';
            avcodec_free_context(&m_codecContext);
            continue;
        }

        // TODO: Isn't this ugly?
        break;
    }

    if (openAudioDevice(audioDevName))
    {
        m_state = STATE_ERROR;
        return OPENERROR_OUTPUT;
    }

    if (initResampleContext())
    {
        m_state = STATE_ERROR;
        return OPENERROR_OTHER;
    }

    // Opening is done
    m_state = STATE_PAUSED;
    return OPENERROR_OK;
}

void Music::unPause()
{
    if (m_state == STATE_ERROR)
    {
        std::cerr << "Failed to unpause, object is in error state" << '\n';
        return;
    }

    if (m_state == STATE_UNINITIALIZED)
    {
        std::cerr << "Failed to unpause, object is uninitialized." << '\n';
        return;
    }

    if (m_formatContext->nb_streams < 1)
    {
        std::cout << "Failed to unpause, no streams found" << '\n';
        m_state = STATE_ERROR;
        return;
    }

    m_state = STATE_PLAYING;
}

void Music::pause()
{
    if (m_state == STATE_ERROR)
    {
        std::cerr << "Failed to pause, object is in error state" << '\n';
        return;
    }

    if (m_state == STATE_UNINITIALIZED)
    {
        std::cerr << "Failed to pause, object is uninitialized." << '\n';
        return;
    }

    m_state = STATE_PAUSED;
}

static AVPacket *resampleFrame(
        AVFrame *frame,
        AVCodecContext *codecContext,
        SwrContext *resampleContext,
        AVStream *outputStream)
{
    uint8_t *resampledBuffer{};
    long outSamples{av_rescale_rnd(
            swr_get_delay(
                    resampleContext,
                    frame->sample_rate) + frame->nb_samples,
            codecContext->sample_rate,
            outputStream->codecpar->sample_rate,
            AV_ROUND_UP)};
    // Allocate buffer for converted data
    av_samples_alloc(
            &resampledBuffer,
            nullptr,
            outputStream->codecpar->channels,
            outSamples,
            (AVSampleFormat)outputStream->codecpar->format,
            0);
    // Do the conversion from the input format to the output format
    swr_convert(
            resampleContext,                       // Resample context
            &resampledBuffer,                      // Output buffer
            outSamples,                            // Number of samples to output
            (const uint8_t**)frame->extended_data, // Input buffer
            frame->nb_samples);                    // Number of input samples

    // We need the buffer size to create a packet from the buffer
    int resampledBufferSize{av_samples_get_buffer_size(
            nullptr,
            outputStream->codecpar->channels,
            outSamples,
            (AVSampleFormat)outputStream->codecpar->format,
            0)};
    if (resampledBufferSize < 0)
    {
        std::cerr << "Failed to get buffer size" << '\n';
        return nullptr;
    }

    AVPacket *resampledPacket{av_packet_alloc()};
    // Create a packet from the buffer, so we can later send it to the device
    if (av_packet_from_data(resampledPacket, resampledBuffer, resampledBufferSize))
    {
        std::cerr << "Failed to create packet from buffer" << '\n';
        return nullptr;
    }

    return resampledPacket;
}

void Music::tick()
{
    switch (m_state)
    {
    case STATE_PLAYING:
        // Continue
        break;

    case STATE_UNINITIALIZED:
        // No file open
        std::cerr << "Failed to tick, uninitialized" << '\n';
        return;

    case STATE_ERROR:
        // An error occurred, probably no stream found
        std::cerr << "Failed to tick, object is in error state" << '\n';
        return;

    case STATE_PAUSED:
        // If paused, don't do anything
        // No break
    case STATE_END:
        // If end of file, don't do anything
        // No break
    default:
        return;
    }

    AVFrame *frame{av_frame_alloc()};
    AVPacket *packet{av_packet_alloc()};

    // Read a frame
    // FIXME: End of stream detection is buggy
    if (av_read_frame(m_formatContext, packet) < 0)
    {
        // End of stream

        std::cout << "End of stream" << '\n';
        av_write_frame(m_outputFormatContext, nullptr); // Flush the buffer
        av_frame_free(&frame);
        av_packet_free(&packet);
        m_state = STATE_END;
        return;
    }

    // Send the packet to the codec
    if (avcodec_send_packet(m_codecContext, packet))
    {
        std::cerr << "Failed to send packet to codec" << '\n';
        return; // Maybe next time
    }

    // Get decoded output data from codec
    // TODO: Handle error depending on the return code
    if (avcodec_receive_frame(m_codecContext, frame))
    {
        std::cerr << "Failed to receive frame from codec" << '\n';
        return; // Maybe next time
    }

    auto resampledFrame{resampleFrame(
            frame,
            m_codecContext,
            m_resampleContext,
            m_outputStream)};
    if (resampledFrame)
    {
        // Write the data to the output device
        if (av_write_frame(m_outputFormatContext, resampledFrame) < 0)
        {
            std::cerr << "Failed to write packet to output device" << '\n';
        }

        av_packet_unref(resampledFrame);
    }
    av_packet_unref(packet);
    av_frame_unref(frame);
}

void Music::closeAndReset()
{
    // TODO: Should we check if they are not invalid/null?
    avformat_close_input(&m_formatContext);
    avcodec_free_context(&m_codecContext);
    swr_free(&m_resampleContext);

    std::cout << "File closed" << '\n';

    reset();
}

Music::~Music()
{
    if (m_state != STATE_UNINITIALIZED)
        closeAndReset();
}
