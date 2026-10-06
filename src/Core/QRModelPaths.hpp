#pragma once

#include <array>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <windows.h>

inline std::filesystem::path QRModelDirectory()
{
    std::vector<wchar_t> path(512);
    while (path.size() <= 32768)
    {
        const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (!length)
        {
            throw std::runtime_error("Cannot resolve executable directory");
        }
        if (length < path.size())
        {
            return std::filesystem::path(std::wstring(path.data(), length)).parent_path() / "ScanModel";
        }
        path.resize(path.size() * 2);
    }
    throw std::runtime_error("Executable path is too long");
}

inline std::array<std::filesystem::path, 4> QRModelPaths(const std::filesystem::path& directory)
{
    std::array<std::filesystem::path, 4> paths{
        directory / "detect.prototxt", directory / "detect.caffemodel",
        directory / "sr.prototxt", directory / "sr.caffemodel"
    };
    for (const auto& path : paths)
    {
        std::error_code error;
        if (!std::filesystem::is_regular_file(path, error) || error ||
            std::filesystem::file_size(path, error) == 0 || error)
        {
            throw std::runtime_error("QR model file is missing, empty, or inaccessible");
        }
    }
    return paths;
}
