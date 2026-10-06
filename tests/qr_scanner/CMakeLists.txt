cmake_minimum_required(VERSION 3.26)
project(qr_scanner_regression LANGUAGES CXX)
find_package(OpenCV REQUIRED COMPONENTS opencv_world)
add_executable(test_qr_scanner test_qr_scanner.cpp ../../src/Core/QRScanner.cpp
    ../../3rdparty/QR-Code-generator/cpp/qrcodegen.cpp)
target_compile_features(test_qr_scanner PRIVATE cxx_std_20)
target_include_directories(test_qr_scanner PRIVATE ../../src/Core ../../3rdparty/QR-Code-generator/cpp)
target_include_directories(test_qr_scanner SYSTEM PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(test_qr_scanner PRIVATE ${OpenCV_LIBS})
if(MSVC)
    target_compile_options(test_qr_scanner PRIVATE /W4 /EHsc /utf-8)
endif()
add_custom_command(TARGET test_qr_scanner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../ScanModel"
            "$<TARGET_FILE_DIR:test_qr_scanner>/ScanModel")
enable_testing()
add_test(NAME qr_scanner_regression COMMAND test_qr_scanner)
