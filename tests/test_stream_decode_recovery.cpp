#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}
#include "StreamTiming.hpp"

namespace
{
void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
struct CodecDeleter { void operator()(AVCodecContext* c) const { avcodec_free_context(&c); } };
struct FrameDeleter { void operator()(AVFrame* f) const { av_frame_free(&f); } };
struct PacketDeleter { void operator()(AVPacket* p) const { av_packet_free(&p); } };
using Codec = std::unique_ptr<AVCodecContext, CodecDeleter>;
using Frame = std::unique_ptr<AVFrame, FrameDeleter>;
using Packet = std::unique_ptr<AVPacket, PacketDeleter>;
}

int main()
{
    try
    {
        av_log_set_level(AV_LOG_QUIET);
        const AVCodec* h264Encoder = avcodec_find_encoder_by_name("libx264");
        const AVCodec* h264Decoder = avcodec_find_decoder(AV_CODEC_ID_H264);
        require(h264Encoder && h264Decoder, "H.264 encoder/decoder missing from matching SDK");
        Codec encoder(avcodec_alloc_context3(h264Encoder));
        Codec decoder(avcodec_alloc_context3(h264Decoder));
        Frame input(av_frame_alloc()), output(av_frame_alloc());
        Packet encoded(av_packet_alloc()), invalid(av_packet_alloc());
        require(encoder && decoder && input && output && encoded && invalid, "allocation failed");
        encoder->width = 64;
        encoder->height = 48;
        encoder->pix_fmt = AV_PIX_FMT_YUV420P;
        encoder->time_base = AVRational{1, 25};
        encoder->gop_size = 1;
        encoder->max_b_frames = 0;
        require(av_opt_set(encoder->priv_data, "preset", "ultrafast", 0) == 0, "preset failed");
        require(av_opt_set(encoder->priv_data, "tune", "zerolatency", 0) == 0, "tune failed");
        require(avcodec_open2(encoder.get(), h264Encoder, nullptr) == 0, "encoder open failed");
        decoder->thread_count = 1;
        require(avcodec_open2(decoder.get(), h264Decoder, nullptr) == 0, "decoder open failed");
        input->width = encoder->width;
        input->height = encoder->height;
        input->format = encoder->pix_fmt;
        require(av_frame_get_buffer(input.get(), 32) == 0, "input buffer failed");
        for (int plane = 0; plane < 3; ++plane)
            for (int y = 0; y < (plane == 0 ? 48 : 24); ++y)
                std::fill_n(input->data[plane] + y * input->linesize[plane],
                            plane == 0 ? 64 : 32, static_cast<uint8_t>(plane == 0 ? 64 : 128));

        std::vector<Packet> packets;
        for (int i = 0; i < 3; ++i)
        {
            input->pts = i;
            require(avcodec_send_frame(encoder.get(), input.get()) == 0, "encode send failed");
            int result;
            while ((result = avcodec_receive_packet(encoder.get(), encoded.get())) == 0)
            {
                packets.emplace_back(av_packet_clone(encoded.get()));
                require(packets.back() != nullptr, "packet clone failed");
                av_packet_unref(encoded.get());
            }
            require(result == AVERROR(EAGAIN), "encode receive failed");
        }
        require(packets.size() == 3, "synthetic keyframes missing");
        require(av_new_packet(invalid.get(), 32) == 0, "invalid packet allocation failed");
        std::fill_n(invalid->data, invalid->size, static_cast<uint8_t>(0xFF));
        StreamDeadline deadline;
        deadline.refresh(0, 10000000);
        int frames = 0;
        for (const auto& packet : packets)
        {
            require(avcodec_send_packet(decoder.get(), packet.get()) == 0, "valid packet rejected");
            int result;
            while ((result = avcodec_receive_frame(decoder.get(), output.get())) == 0)
            {
                require(output->width == 64 && output->height == 48, "recovered frame invalid");
                ++frames;
                av_frame_unref(output.get());
            }
            require(result == AVERROR(EAGAIN), "valid frame receive failed");
            const int corrupt = avcodec_send_packet(decoder.get(), invalid.get());
            require(corrupt == AVERROR_INVALIDDATA && corrupt == -1094995529,
                    "corrupt packet did not reproduce the user's error");
            // The same decoder must survive; do not flush/recreate it or reset the deadline.
        }
        require(frames == 3, "decoder did not resume after malformed packets");
        for (int64_t now = 100000; now < 10000000; now += 100000)
        {
            require(avcodec_send_packet(decoder.get(), invalid.get()) == AVERROR_INVALIDDATA,
                    "repeated corrupt input returned unexpected status");
            require(!deadline.expired(now), "deadline fired early");
        }
        require(deadline.expired(10000000), "corrupt-only input bypassed timeout");
        std::cout << "real H.264 valid/corrupt/valid recovery and corrupt-only deadline passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
