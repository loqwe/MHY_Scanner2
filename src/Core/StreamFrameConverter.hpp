#pragma once

#include <cerrno>
#include <cstdint>

extern "C"
{
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

// The caller owns the scaler and an output buffer of at least dstStride * dstHeight bytes.
inline int ConvertStreamFrame(SwsContext*& scaler, const AVFrame& frame,
                              int dstWidth, int dstHeight, uint8_t* dst, int dstStride)
{
    const auto format = static_cast<AVPixelFormat>(frame.format);
    const auto* descriptor = av_pix_fmt_desc_get(format);
    if (!descriptor || !sws_isSupportedInput(format) || !dst ||
        frame.width <= 0 || frame.height <= 0 || dstWidth <= 0 || dstHeight <= 0 ||
        av_image_check_size(frame.width, frame.height, 0, nullptr) < 0 ||
        av_image_check_size(dstWidth, dstHeight, 0, nullptr) < 0)
    {
        return AVERROR(EINVAL);
    }
    const int outputStride = av_image_get_linesize(AV_PIX_FMT_BGR24, dstWidth, 0);
    if (outputStride <= 0 || dstStride < outputStride)
    {
        return AVERROR(EINVAL);
    }
    int sourceStrides[4]{};
    if (av_image_fill_linesizes(sourceStrides, format, frame.width) < 0)
    {
        return AVERROR(EINVAL);
    }
    for (int plane = 0; plane < 4; ++plane)
    {
        const int64_t stride = frame.linesize[plane];
        if (sourceStrides[plane] > 0 &&
            (!frame.data[plane] || (stride < 0 ? -stride : stride) < sourceStrides[plane]))
        {
            return AVERROR(EINVAL);
        }
    }
    if ((descriptor->flags & AV_PIX_FMT_FLAG_PAL) && !frame.data[1])
    {
        return AVERROR(EINVAL);
    }
    scaler = sws_getCachedContext(scaler, frame.width, frame.height, format,
                                 dstWidth, dstHeight, AV_PIX_FMT_BGR24,
                                 SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!scaler)
    {
        return AVERROR(ENOMEM);
    }
    uint8_t* dstData[4] = { dst, nullptr, nullptr, nullptr };
    const int dstLinesize[4] = { dstStride, 0, 0, 0 };
    return sws_scale(scaler, frame.data, frame.linesize, 0, frame.height, dstData, dstLinesize);
}
