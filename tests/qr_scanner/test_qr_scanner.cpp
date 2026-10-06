#include "QRScanner.h"
#include "qrcodegen.hpp"

#include <iostream>
#include <stdexcept>

int main()
{
    try
    {
        const std::string expected = "https://example.invalid/login?ticket=regression-only";
        const auto code = qrcodegen::QrCode::encodeText(expected.c_str(), qrcodegen::QrCode::Ecc::MEDIUM);
        constexpr int border = 4;
        constexpr int scale = 8;
        const int size = (code.getSize() + border * 2) * scale;
        cv::Mat image(size, size, CV_8UC3, cv::Scalar(255, 255, 255));
        for (int y = 0; y < code.getSize(); ++y)
        {
            for (int x = 0; x < code.getSize(); ++x)
            {
                if (code.getModule(x, y))
                {
                    image(cv::Rect((x + border) * scale, (y + border) * scale, scale, scale))
                        .setTo(cv::Scalar(0, 0, 0));
                }
            }
        }
        QRScanner scanner;
        std::string decoded;
        scanner.decodeSingle(image, decoded);
        if (decoded != expected) throw std::runtime_error("real QR decode failed");
        scanner.decodeSingle({}, decoded);
        if (!decoded.empty()) throw std::runtime_error("empty image retained old QR");
        scanner.decodeMultiple(image, decoded);
        if (decoded != expected) throw std::runtime_error("multiple QR decode failed");
        scanner.decodeMultiple({}, decoded);
        if (!decoded.empty()) throw std::runtime_error("empty multi image retained old QR");
        std::cout << "real model initialization, QR decoding and stale-output clearing passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
