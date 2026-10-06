#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "StreamFrameConverter.hpp"

namespace
{
void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

struct FrameDeleter
{
    void operator()(AVFrame* frame) const { av_frame_free(&frame); }
};

std::unique_ptr<AVFrame, FrameDeleter> makeFrame(int width, int height, AVPixelFormat format)
{
    std::unique_ptr<AVFrame, FrameDeleter> frame(av_frame_alloc());
    require(frame != nullptr, "frame allocation failed");
    frame->width = width;
    frame->height = height;
    frame->format = format;
    require(av_frame_get_buffer(frame.get(), 32) == 0, "frame buffer allocation failed");
    return frame;
}
}

int main()
{
    SwsContext* scaler = nullptr;
    try
    {
        std::vector<uint8_t> output(64 * 48 * 3 + 64, 0xA5);
        AVFrame invalid{};
        invalid.width = 64;
        invalid.height = 48;
        invalid.format = AV_PIX_FMT_NONE;
        require(ConvertStreamFrame(scaler, invalid, 64, 48, output.data(), 64 * 3) < 0,
                "unknown pixel format was not rejected");
        require(scaler == nullptr, "invalid input created a scaler");

        invalid.format = AV_PIX_FMT_YUV420P;
        invalid.width = 0;
        require(ConvertStreamFrame(scaler, invalid, 64, 48, output.data(), 64 * 3) < 0,
                "zero input width was not rejected");
        invalid.width = 64;
        require(ConvertStreamFrame(scaler, invalid, 64, 48, output.data(), 64 * 3) < 0,
                "missing source planes were not rejected");

        auto rgb = makeFrame(64, 48, AV_PIX_FMT_RGB24);
        for (int y = 0; y < rgb->height; ++y)
        {
            for (int x = 0; x < rgb->width; ++x)
            {
                auto* pixel = rgb->data[0] + y * rgb->linesize[0] + x * 3;
                pixel[0] = 255;
                pixel[1] = 0;
                pixel[2] = 0;
            }
        }
        require(ConvertStreamFrame(scaler, *rgb, 64, 48, output.data(), 64 * 3) == 48,
                "RGB conversion failed");
        require(output[0] == 0 && output[1] == 0 && output[2] == 255,
                "RGB to BGR output is incorrect");
        require(std::all_of(output.begin() + 64 * 48 * 3, output.end(),
                            [](uint8_t value) { return value == 0xA5; }),
                "conversion wrote beyond the output buffer");

        require(ConvertStreamFrame(scaler, *rgb, 64, 48, output.data(), 64 * 3 - 1) < 0,
                "undersized destination stride was not rejected");
        require(ConvertStreamFrame(scaler, *rgb, 64, 48, nullptr, 64 * 3) < 0,
                "null output was not rejected");
        require(ConvertStreamFrame(scaler, *rgb, 0, 48, output.data(), 64 * 3) < 0,
                "zero output width was not rejected");
        require(ConvertStreamFrame(scaler, invalid, 64, 48, output.data(), 64 * 3) < 0,
                "invalid input after a valid frame was not rejected");
        require(ConvertStreamFrame(scaler, *rgb, 64, 48, output.data(), 64 * 3) == 48,
                "scaler did not recover after invalid input");

        auto yuv = makeFrame(32, 24, AV_PIX_FMT_YUV420P);
        for (int plane = 0; plane < 3; ++plane)
        {
            const int rows = plane == 0 ? 24 : 12;
            for (int y = 0; y < rows; ++y)
            {
                std::fill_n(yuv->data[plane] + y * yuv->linesize[plane],
                            plane == 0 ? 32 : 16, static_cast<uint8_t>(plane == 0 ? 16 : 128));
            }
        }
        require(ConvertStreamFrame(scaler, *yuv, 16, 12, output.data(), 16 * 3) == 12,
                "resolution and pixel format change failed");
        require(output[0] < 3 && output[1] < 3 && output[2] < 3,
                "YUV black frame converted incorrectly");
        require(ConvertStreamFrame(scaler, *rgb, 64, 48, output.data(), 64 * 3) == 48,
                "returning to the original format failed");

        const auto savedFormat = rgb->format;
        rgb->format = AV_PIX_FMT_NB + 100;
        require(ConvertStreamFrame(scaler, *rgb, 64, 48, output.data(), 64 * 3) < 0,
                "out-of-range pixel format was not rejected");
        rgb->format = savedFormat;
        const auto savedStride = rgb->linesize[0];
        rgb->linesize[0] = 1;
        require(ConvertStreamFrame(scaler, *rgb, 64, 48, output.data(), 64 * 3) < 0,
                "undersized source stride was not rejected");
        rgb->linesize[0] = savedStride;

        sws_freeContext(scaler);
        std::cout << "PASS: invalid input, BGR output, buffer bounds, format changes, recovery\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        sws_freeContext(scaler);
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
