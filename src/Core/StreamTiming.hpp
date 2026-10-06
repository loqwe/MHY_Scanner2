#pragma once

#include <atomic>
#include <cstdint>

class StreamDeadline
{
public:
    void refresh(std::int64_t now, std::int64_t timeout)
    {
        deadline.store(now + timeout);
    }

    bool expired(std::int64_t now) const
    {
        return now >= deadline.load();
    }

private:
    std::atomic<std::int64_t> deadline{0};
};

class StreamSubmitClock
{
public:
    void reset() { next = 0; }
    bool due(std::int64_t now) const { return now >= next; }
    void submitted(std::int64_t now) { next = now + 200000; }

private:
    std::int64_t next{0};
};
