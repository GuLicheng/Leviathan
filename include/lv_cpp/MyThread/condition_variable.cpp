#include "condition_variable.hpp"
#include <iostream>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

namespace test1
{

    ConditionVariable cv;
    Mutex cv_m;
    int i;

    void waits(int idx)
    {
        UniqueLock<Mutex> lk(cv_m);
        if (cv.wait_for(lk, idx * 100ms, []
                        { return i == 1; }))
            std::cerr << "Thread " << idx << " finished waiting. i == " << i << '\n';
        else
            std::cerr << "Thread " << idx << " timed out. i == " << i << '\n';
    }

    void signals()
    {
        std::this_thread::sleep_for(120ms);
        std::cerr << "Notifying...\n";
        cv.notify_all();
        std::this_thread::sleep_for(100ms);
        {
            LockGuard<Mutex> lk(cv_m);
            i = 1;
        }
        std::cerr << "Notifying again...\n";
        cv.notify_all();
    }

    int Main()
    {
        std::thread t1(waits, 1), t2(waits, 2), t3(waits, 3), t4(signals);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        return 0;
    }

}

namespace test2
{
    ConditionVariable cv;
    Mutex cv_m; // This mutex is used for three purposes:
                // 1) to synchronize accesses to i
                // 2) to synchronize accesses to std::cerr
                // 3) for the condition variable cv
    int i = 0;

    void waits()
    {
        UniqueLock<Mutex> lk(cv_m);
        std::cerr << "Waiting... \n";
        cv.wait(lk, []
                { return i == 1; });
        std::cerr << "...finished waiting. i == 1\n";
    }

    void signals()
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        {
            LockGuard<Mutex> lk(cv_m);
            std::cerr << "Notifying...\n";
        }
        cv.notify_all();

        std::this_thread::sleep_for(std::chrono::seconds(1));

        {
            LockGuard<Mutex> lk(cv_m);
            i = 1;
            std::cerr << "Notifying again...\n";
        }
        cv.notify_all();
    }

    int Main()
    {
        std::thread t1(waits), t2(waits), t3(waits), t4(signals);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        return 0;
    }
}

namespace test3
{
    ConditionVariable cv;
    Mutex cv_m;
    std::atomic<int> i{0};

    void waits(int idx)
    {
        UniqueLock<Mutex> lk(cv_m);
        auto now = std::chrono::system_clock::now();
        if (cv.wait_until(lk, now + idx * 100ms, []()
                          { return i == 1; }))
            std::cerr << "Thread " << idx << " finished waiting. i == " << i << '\n';
        else
            std::cerr << "Thread " << idx << " timed out. i == " << i << '\n';
    }

    void signals()
    {
        std::this_thread::sleep_for(120ms);
        std::cerr << "Notifying...\n";
        cv.notify_all();
        std::this_thread::sleep_for(100ms);
        i = 1;
        std::cerr << "Notifying again...\n";
        cv.notify_all();
    }

    int Main()
    {
        std::thread t1(waits, 1), t2(waits, 2), t3(waits, 3), t4(signals);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        return 0;
    }
}

int main()
{
    test1::Main();
    test2::Main();
    test3::Main();
}
