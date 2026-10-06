#include "StreamTiming.hpp"

#include <iostream>
#include <stdexcept>

void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

int main()
{
    try
    {
        StreamDeadline deadline;
        deadline.refresh(100, 1000);
        require(!deadline.expired(1099), "deadline fired early");
        require(deadline.expired(1100), "deadline boundary missed");
        deadline.refresh(1000, 1000);
        require(!deadline.expired(1100), "video frame did not refresh deadline");
        require(deadline.expired(2000), "refreshed deadline missed");

        StreamSubmitClock clock;
        require(clock.due(100), "first frame was throttled");
        clock.submitted(100);
        require(!clock.due(200099), "submission interval is less than 200ms");
        require(clock.due(200100), "submission boundary missed");
        // Failed tryStart must not advance the clock.
        require(clock.due(200101), "busy worker suppressed retry");
        clock.submitted(200101);
        require(!clock.due(200102), "successful submit did not throttle");
        clock.reset();
        require(clock.due(200102), "restart retained previous throttle state");
        std::cout << "stream timing tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
