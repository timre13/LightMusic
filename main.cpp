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
#include "av_includes.h"
#include <ao/ao.h>

#define VERSION_STR "pre-alpha 1.0-dev"

//#define FILENAME "nonpublic-test-music/des.mp3"
//#define FILENAME "nonpublic-test-music/csp.mp3"
#define FILENAME "nonpublic-test-music/csp.wav"
//#define FILENAME "nonpublic-test-music/csp_short.wav"
//#define FILENAME "nonpublic-test-music/csp.ogg"
//#define FILENAME "nonpublic-test-music/blocky.mkv"

int main()
{
    std::cout << "LightMusic music player version " VERSION_STR << '\n';

    // Init the audio output library
    ao_initialize();

    AVFormatContext *formatContext{avformat_alloc_context()};
    if (!formatContext)
    {
        std::cerr << "Failed to allocate context" << '\n';
        abort();
    }

    // Open the filename as input to the format context
    if (avformat_open_input(&formatContext, FILENAME, nullptr, nullptr))
    {
        std::cerr << "Failed to open file" << '\n';
        abort();
    }

    // Print some info
    std::cout << "Filename/url: " << formatContext->url << '\n';
    std::cout << "Metadata:" << '\n';
    AVDictionaryEntry *metadataTag{nullptr};
    while ((metadataTag = av_dict_get(formatContext->metadata, "", metadataTag, AV_DICT_IGNORE_SUFFIX)))
        std::cout << "\t" << metadataTag->key << ": " << metadataTag->value << '\n';
    std::cout << "Format: " << formatContext->iformat->long_name << '\n';
    std::cout << "Duration: " << (long double)formatContext->duration / AV_TIME_BASE << "s" << '\n';
    std::cout << "Bit rate: " << formatContext->bit_rate << '\n';

    // Find the info of the streams, so we can use them
    if (avformat_find_stream_info(formatContext, nullptr))
    {
        std::cerr << "Failed to find stream info" << '\n';
        abort();
    }

    std::cout << "Number of streams: " << formatContext->nb_streams << '\n';

    // Loop through the streams
    for (unsigned int i{}; i < formatContext->nb_streams; ++i)
    {
        auto codecParameters{formatContext->streams[i]->codecpar};
        // Find a codec for the stream
        auto codec{avcodec_find_decoder(codecParameters->codec_id)};
        if (!codec)
        {
            std::cerr << "Failed to find decoder for stream, skipping" << '\n';
            continue;
        }

        // Print info of the stream
        std::cout << "Stream #" << i << ":" << '\n';
        std::cout << "\tCodec name: " << codec->long_name << '\n';
        std::cout << "\tCodec ID: " << codec->id << '\n';
        std::cout << "\tBit rate: " << codecParameters->bit_rate << '\n';

        // If this is an audio stream
        if (codec->type == AVMEDIA_TYPE_AUDIO)
        {
            std::cout << "\tChannels: " << codecParameters->channels << '\n';
            std::cout << "\tSample rate: " << codecParameters->sample_rate << '\n';

            auto codecContext{avcodec_alloc_context3(codec)};
            if (!codecContext)
            {
                std::cerr << "Failed to allocate codec context, skipping" << '\n';
                continue;
            }

            if (avcodec_parameters_to_context(codecContext, codecParameters))
            {
                std::cerr << "Failed to fill codex context, skipping" << '\n';
                avcodec_free_context(&codecContext);
                continue;
            }

            char sampleFormatStr[32];
            std::cout << "\tSample format: " << av_get_sample_fmt_string(sampleFormatStr, sizeof(sampleFormatStr), codecContext->sample_fmt) << '\n';

            ao_sample_format aoSampleFormat{};
            switch (codecContext->sample_fmt)
            {
            case AV_SAMPLE_FMT_U8:
                aoSampleFormat.bits = 8;
                break;

            case AV_SAMPLE_FMT_S16:
                aoSampleFormat.bits = 16;
                break;

            case AV_SAMPLE_FMT_S32:
                aoSampleFormat.bits = 32;
                break;

            case AV_SAMPLE_FMT_U8P:
                aoSampleFormat.bits = 8;
                break;

            case AV_SAMPLE_FMT_S16P:
                aoSampleFormat.bits = 16;
                break;
            case AV_SAMPLE_FMT_S32P:
                aoSampleFormat.bits = 32;
                break;

            case AV_SAMPLE_FMT_S64:
                aoSampleFormat.bits = 64;
                break;

            case AV_SAMPLE_FMT_S64P:
                aoSampleFormat.bits = 64;
                break;

            // TODO: Handle other sample formats, too

            default:
                aoSampleFormat.bits = 16;
            }
            aoSampleFormat.channels = codecContext->channels;
            aoSampleFormat.rate = codecContext->sample_rate;
            aoSampleFormat.byte_format = AO_FMT_NATIVE;
            aoSampleFormat.matrix = nullptr;

            // Open the default audio device
            auto audioDev{ao_open_live(ao_default_driver_id(), &aoSampleFormat, nullptr)};
            if (!audioDev)
            {
                std::cerr << "Failed to open audio device: " << strerror(errno) << '\n';
                abort();
            }

            if (avcodec_open2(codecContext, codec, nullptr))
            {
                std::cerr << "Failed to open codex context, skipping" << '\n';
                avcodec_free_context(&codecContext);
                continue;
            }

            //int bufferSize{1};
            //uint8_t buffer[bufferSize];
            AVFrame *frame{av_frame_alloc()};
            AVPacket *packet{av_packet_alloc()};

            while (av_read_frame(formatContext, packet) >= 0)
            {
                // Send the packet to the codec
                if (avcodec_send_packet(codecContext, packet))
                {
                    std::cerr << "Failed to send packet to codec" << '\n';
                    continue;
                }

                // Get decoded output data from codec
                // TODO: Handle error depending on the return code
                if (avcodec_receive_frame(codecContext, frame))
                {
                    std::cerr << "Failed to receive frame from codec" << '\n';
                    continue;
                }

                // Play the buffer
                ao_play(audioDev, (char*)frame->data[0], frame->linesize[0]);

                // Free the packet
                av_packet_unref(packet);
            }

            std::cout << "End of stream" << '\n';

            avcodec_free_context(&codecContext);

            ao_close(audioDev); // Close the audio device
        }
        else
        {
            std::cerr << "\tNot an audio stream, skipping" << '\n';
        }
    }
    std::cout << "End of container" << '\n';

    ao_shutdown();
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    std::cout << "========== Exit ==========" << '\n';

    return 0;
}
