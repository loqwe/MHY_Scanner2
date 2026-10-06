#include "ConfirmationDelay.hpp"

#include <iostream>
#include <limits>
#include <stdexcept>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFuture>

void require(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}

void processFor(int milliseconds)
{
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    try
    {
        require(!QFuture<void>().isRunning(), "default future blocks initial confirmation");
        require(ConfirmationDelayMs(0) == 0, "zero default changed");
        require(ConfirmationDelayMs(1.5) == 1500, "fractional seconds lost");
        require(ConfirmationDelayMs(-1) == 0, "negative delay allowed");
        require(ConfirmationDelayMs(120) == 60000, "maximum delay not bounded");
        require(ConfirmationDelayMs(std::numeric_limits<double>::infinity()) == 0, "infinite delay allowed");
        ConfirmationDelay delay;
        int called = 0;
        QElapsedTimer elapsed;
        elapsed.start();
        delay.schedule(0.1, [&]() { ++called; require(elapsed.elapsed() >= 100, "confirmation ran early"); });
        require(delay.pending(), "delay not active");
        processFor(50);
        require(called == 0, "confirmation executed before delay");
        processFor(100);
        require(called == 1 && !delay.pending(), "confirmation did not execute once");
        delay.schedule(0.1, [&]() { ++called; });
        delay.cancel();
        processFor(150);
        require(called == 1, "cancelled confirmation executed");
        delay.schedule(0.1, [&]() { ++called; });
        delay.schedule(0, [&]() { called += 10; });
        processFor(150);
        require(called == 11, "stale confirmation survived replacement");
        {
            ConfirmationDelay destroyed;
            destroyed.schedule(0.1, [&]() { ++called; });
        }
        processFor(150);
        require(called == 11, "destroyed confirmation executed");
        std::cout << "confirmation timing, cancellation, replacement and destruction passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
