#include "StreamTiming.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

struct InterruptState
{
    StreamDeadline deadline;
    std::atomic<bool> running{true};
    std::atomic<bool> interrupted{false};

    static int callback(void* opaque)
    {
        auto& state = *static_cast<InterruptState*>(opaque);
        if (!state.running.load() || state.deadline.expired(av_gettime_relative()))
        {
            state.interrupted.store(true);
            return 1;
        }
        return 0;
    }
};

int main(int argc, char** argv)
{
    if (argc != 3) return 2;
    av_log_set_level(AV_LOG_FATAL);
    InterruptState state;
    const bool cancel = std::string(argv[2]) == "cancel";
    state.deadline.refresh(av_gettime_relative(), cancel ? 5000000 : 150000);
    AVFormatContext* context = avformat_alloc_context();
    if (!context) return 3;
    context->interrupt_callback = { &InterruptState::callback, &state };
    std::thread stopper;
    if (cancel)
    {
        stopper = std::thread([&state]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            state.running.store(false);
        });
    }
    const auto start = av_gettime_relative();
    const int result = avformat_open_input(&context, argv[1], nullptr, nullptr);
    const auto elapsed = av_gettime_relative() - start;
    if (stopper.joinable()) stopper.join();
    avformat_close_input(&context);
    if (result >= 0 || !state.interrupted.load() || elapsed < 100000 || elapsed > 2000000)
    {
        std::cerr << "interrupt failed result=" << result << " elapsed_us=" << elapsed << '\n';
        return 1;
    }
    std::cout << argv[2] << " interrupted blocked read in " << elapsed << "us\n";
    return 0;
}
