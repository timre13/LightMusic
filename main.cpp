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

#define VERSION_STR "pre-alpha 1.0-dev"
#define AUDIO_OUTPUT_DEVICE_NAME "alsa" // TODO: Windows compatibility

//#define FILENAME "nonpublic-test-music/des.mp3"
//#define FILENAME "nonpublic-test-music/csp.mp3"
#define FILENAME "nonpublic-test-music/csp.wav"
//#define FILENAME "nonpublic-test-music/csp_short.wav"
//#define FILENAME "nonpublic-test-music/csp.ogg"
//#define FILENAME "nonpublic-test-music/blocky.mkv"

int main()
{
    std::cout << "LightMusic music player version " VERSION_STR << '\n';

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

    // ------------------------------------------------------------------------

    // Init audio I/O
    avdevice_register_all();


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

            if (avcodec_open2(codecContext, codec, nullptr))
            {
                std::cerr << "Failed to open codex context, skipping" << '\n';
                avcodec_free_context(&codecContext);
                continue;
            }

            // Get the output format of the audio device
            AVOutputFormat *outputFormat{av_guess_format(AUDIO_OUTPUT_DEVICE_NAME, nullptr, nullptr)};
            if (!outputFormat)
            {
                std::cerr << "Failed to get format of output device" << '\n';
                abort();
            }

            AVFormatContext *outputFormatContext{avformat_alloc_context()};
            if (!outputFormatContext)
            {
                std::cerr << "Failed to create output format context" << '\n';
                abort();
            }
            // Tell the format context which output device to use
            outputFormatContext->oformat = outputFormat;

            // Create a stream where the output will be written to
            AVStream* outputStream{avformat_new_stream(outputFormatContext, nullptr)};
            if (!outputStream)
            {
                std::cerr << "Failed to create output stream" << '\n';
                abort();
            }
            // Configure the parameters of the output stream
            outputStream->codecpar->codec_id       = AV_CODEC_ID_PCM_S16LE;
            outputStream->codecpar->codec_type     = AVMEDIA_TYPE_AUDIO;
            outputStream->codecpar->format         = AV_SAMPLE_FMT_S16;
            outputStream->codecpar->bit_rate       = codecContext->bit_rate;
            outputStream->codecpar->sample_rate    = codecContext->sample_rate;
            outputStream->codecpar->channels       = codecContext->channels;
            outputStream->codecpar->channel_layout = AV_CH_LAYOUT_STEREO;

            // Tell the device the parameters
            if (avformat_write_header(outputFormatContext, nullptr) < 0)
            {
                std::cerr << "Failed to write stream header to output device" << '\n';
                abort();
            }

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

                SwrContext *resampleContext{
                        swr_alloc_set_opts(
                                nullptr,
                                outputStream->codecpar->channel_layout,         // Out channel layout
                                (AVSampleFormat)outputStream->codecpar->format, // Out sample format
                                outputStream->codecpar->sample_rate,            // Out sample rate
                                AV_CH_LAYOUT_STEREO,                            // In channel layout
                                (AVSampleFormat)codecContext->sample_fmt,       // In format
                                codecContext->sample_rate,                      // In sample rate
                                0,
                                nullptr
                )};
                swr_init(resampleContext);

                uint8_t *resampledBuffer{};
                int outSamples{av_rescale_rnd(
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
                swr_convert(resampleContext, &resampledBuffer, outSamples, (const uint8_t**)frame->extended_data, frame->nb_samples);

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
                    continue;
                }

                AVPacket *resampledPacket{av_packet_alloc()};
                // Create a packet from the buffer, so we can later send it to the device
                if (av_packet_from_data(resampledPacket, resampledBuffer, resampledBufferSize))
                {
                    std::cerr << "Failed to create packet from buffer" << '\n';
                    continue;
                }

                // Write the data to the output device
                if (av_write_frame(outputFormatContext, resampledPacket) < 0)
                {
                    std::cerr << "Failed to write packet to output device" << '\n';
                }

                // Free stuff
                av_packet_unref(packet);
                av_frame_unref(frame);
                av_packet_unref(resampledPacket);
                swr_free(&resampleContext);
            }

            std::cout << "\tEnd of stream" << '\n';

            avcodec_free_context(&codecContext);
        }
        else
        {
            std::cerr << "\tNot an audio stream, skipping" << '\n';
        }
    }
    std::cout << "End of container" << '\n';

    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    std::cout << "========== Exit ==========" << '\n';

    return 0;
}
