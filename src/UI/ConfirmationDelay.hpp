#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <utility>

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

inline int ConfirmationDelayMs(double seconds)
{
    if (!std::isfinite(seconds)) return 0;
    return static_cast<int>(std::round(std::clamp(seconds, 0.0, 60.0) * 1000.0));
}

class ConfirmationDelay final : public QObject
{
public:
    explicit ConfirmationDelay(QObject* parent = nullptr) : QObject(parent), timer(this)
    {
        timer.setSingleShot(true);
        timer.setTimerType(Qt::PreciseTimer);
        QObject::connect(&timer, &QTimer::timeout, this, [this]() {
            const auto remaining = milliseconds - elapsed.elapsed();
            if (remaining > 0)
            {
                timer.start(static_cast<int>(remaining));
                return;
            }
            auto action = std::move(callback);
            callback = {};
            if (action) action();
        });
    }

    void schedule(double seconds, std::function<void()> action)
    {
        cancel();
        callback = std::move(action);
        milliseconds = ConfirmationDelayMs(seconds);
        elapsed.start();
        timer.start(milliseconds);
    }

    void cancel()
    {
        timer.stop();
        callback = {};
    }

    bool pending() const { return timer.isActive(); }

private:
    QTimer timer;
    QElapsedTimer elapsed;
    int milliseconds{0};
    std::function<void()> callback;
};
