#pragma once

#include <cstddef>
#include <cstring>
#include <limits>

inline bool CopyScreenBGRA(void* destination, std::size_t capacity, const void* source,
    std::size_t sourcePitch, std::size_t width, std::size_t height)
{
    const auto maximum = (std::numeric_limits<std::size_t>::max)();
    if (!destination || !source || !width || !height || width > maximum / 4)
    {
        return false;
    }
    const std::size_t rowBytes = width * 4;
    if (sourcePitch < rowBytes || height > maximum / rowBytes ||
        capacity < rowBytes * height || height > maximum / sourcePitch)
    {
        return false;
    }
    auto* output = static_cast<unsigned char*>(destination);
    const auto* input = static_cast<const unsigned char*>(source);
    for (std::size_t row = 0; row < height; ++row)
    {
        std::memcpy(output + row * rowBytes, input + row * sourcePitch, rowBytes);
    }
    return true;
}
