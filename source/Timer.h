#ifndef _TIMER_H
#define _TIMER_H

#include <thread>
#include <chrono>
#include <functional>

class Timer
{
public:
    static const long Infinite = -1L;

    Timer() {}

    Timer(const std::function<void()> &f) : funct(f) {}

    Timer(const std::function<void()>& f, 
          const unsigned long &i,
          const long repeat = Timer::Infinite) 
        : funct(f)
        , interval(std::chrono::milliseconds(i))
        , CallNumber(repeat)
    {}

    void Start(bool Async = true)
    {
        if (Alive)
            return;
        Alive = true;
        repeat_count = CallNumber;

        if (Async)
            Thread = std::thread(&Timer::ThreadFunc, this);
        else
            ThreadFunc();
    }

    void Stop()
    {
        Alive = false;
        Thread.join();
    }

    void SetFunction(const std::function<void()> &f) { funct = f; }

    bool IsAlive() const { return Alive; }

    long RepeatCount() const { return CallNumber; }

    long GetLeftCount() const { return repeat_count; }

    void RepeatCount(const long r)
    {
        if (Alive)
            return;
        CallNumber = r;
    }

    void SetInterval(const unsigned long &i)
    {
        if (Alive)
            return;
        interval = std::chrono::milliseconds(i);
    }

    unsigned long long Interval() const { return interval.count(); }

    const std::function<void()>& Function() const { return funct; }

private:
    void SleepAndRun()
    {
        std::this_thread::sleep_for(interval);
        if (Alive)
            Function()();
    }

    void ThreadFunc()
    {
        if (CallNumber == Infinite)
            while (Alive)
                SleepAndRun();
        else
            while (repeat_count--)
                SleepAndRun();
    }

private:
    std::thread Thread;
    bool Alive = false;
    long CallNumber = -1L;
    long repeat_count = -1L;
    std::chrono::milliseconds interval = std::chrono::milliseconds(0);
    std::function<void()> funct = nullptr;
};

#endif // !_TIMER_H
