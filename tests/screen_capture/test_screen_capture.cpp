#include <array>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "QRModelPaths.hpp"
#include "ScreenShotDXGI.hpp"

void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

int main(int argc, char** argv)
{
    try
    {
        const std::array<unsigned char, 24> source{
            1, 2, 3, 4, 5, 6, 7, 8, 99, 99, 99, 99,
            9, 10, 11, 12, 13, 14, 15, 16, 99, 99, 99, 99
        };
        std::array<unsigned char, 18> destination{};
        destination.fill(42);
        require(CopyScreenBGRA(destination.data() + 1, 16, source.data(), 12, 2, 2), "Padded copy failed");
        for (std::size_t i = 0; i < 16; ++i)
        {
            require(destination[i + 1] == i + 1, "RowPitch padding entered image");
        }
        require(destination.front() == 42 && destination.back() == 42, "Copy crossed destination bounds");
        require(!CopyScreenBGRA(destination.data(), 15, source.data(), 12, 2, 2), "Small capacity accepted");
        require(!CopyScreenBGRA(destination.data(), 18, source.data(), 7, 2, 2), "Small pitch accepted");
        require(!CopyScreenBGRA(nullptr, 18, source.data(), 12, 2, 2), "Null output accepted");
        require(!CopyScreenBGRA(destination.data(), 18, nullptr, 12, 2, 2), "Null input accepted");
        require(!CopyScreenBGRA(destination.data(), 18, source.data(), 12, 0, 2), "Zero width accepted");
        const auto maximum = (std::numeric_limits<std::size_t>::max)();
        require(!CopyScreenBGRA(destination.data(), 18, source.data(), maximum, maximum, 2), "Width overflow accepted");
        require(!CopyScreenBGRA(destination.data(), maximum, source.data(), maximum, 1, 2), "Pitch overflow accepted");

        ScreenShotDXGI capture;
        int width = 1;
        int height = 1;
        require(!capture.InitDupl(0, width, height), "Duplication accepted missing device");
        require(width == 0 && height == 0, "Failure left stale dimensions");
        require(capture.getFrame() == 1, "Acquisition accepted missing duplication");
        require(!capture.copyFrameToBuffer(destination.data(), destination.size()), "Copy accepted missing frame");
        require(capture.doneWithFrame(), "Release with no held frame failed");

        const auto modelDirectory = QRModelDirectory();
        const auto workingDirectory = std::filesystem::current_path();
        std::filesystem::current_path(modelDirectory.parent_path());
        require(QRModelDirectory() == modelDirectory, "Models depend on working directory");
        std::filesystem::current_path(workingDirectory);
        bool missingRejected = false;
        try
        {
            QRModelPaths(modelDirectory / "missing-test-models");
        }
        catch (const std::runtime_error&)
        {
            missingRejected = true;
        }
        require(missingRejected, "Missing models silently accepted");
        if (argc == 2)
        {
            require(QRModelPaths(std::filesystem::path(argv[1])).size() == 4, "Valid model files rejected");
        }
        std::cout << "Screen capture regressions passed: padded rows, bounds, overflow, invalid DXGI state, EXE model path, missing models\n";
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}
