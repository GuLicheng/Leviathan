#include <iostream>
#include "mutex.hpp"
#include "lock.hpp"
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <iostream>
#include <atomic>
#include <thread>
#include <future>

namespace try_lock_for
{
    Mutex cout_mutex;
    TimedMutex mutex;

    using namespace std::chrono_literals;

    void job(int id)
    {
        std::ostringstream stream;
        for (int i = 0; i < 3; ++i)
        {
            if (mutex.try_lock_for(100ms))
            {
                stream << "success ";
                std::this_thread::sleep_for(100ms);
                mutex.unlock();
            }
            else
            {
                stream << "failed ";
            }
            std::this_thread::sleep_for(100ms);
        }

        LockGuard<Mutex> lock(cout_mutex);
        std::cout << "[" << id << "] " << stream.str() << std::endl;
    }

    void Main()
    {
        // try_lock_for
        std::vector<std::thread> threads;
        for (int i = 0; i < 4; ++i)
        {
            threads.emplace_back(job, i);
        }

        for (auto &i : threads)
        {
            i.join();
        }
    }
}
namespace try_lock_until
{

    TimedMutex test_mutex;
    using namespace std::chrono;

    void f()
    {
        auto now = steady_clock::now();
        if (test_mutex.try_lock_until(now + 3s))
        {
            std::cout << "get lock." << std::endl;
        }
        else
        {
            std::cout << "try_lock_util timeout." << std::endl;
        }
    }

    void Main()
    {
        // try_lock_until
        auto start = steady_clock::now();
        LockGuard<TimedMutex> l(test_mutex);
        std::thread t(f);
        t.join();
        auto end = steady_clock::now();
        std::cout << "time use: " << duration_cast<milliseconds>(end - start).count()
                  << "ms." << std::endl;
    }

}
namespace recursive_mutex
{

    int g_num = 0; // protected by g_num_mutex
    Mutex g_num_mutex;

    void slow_increment(int id)
    {
        for (int i = 0; i < 3; ++i)
        {
            g_num_mutex.lock();
            ++g_num;
            std::cout << id << " => " << g_num << '\n';
            g_num_mutex.unlock();

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void Main()
    {
        std::thread t1(slow_increment, 0);
        std::thread t2(slow_increment, 1);
        t1.join();
        t2.join();
    }
}
int main()
{
    try_lock_for::Main();
    try_lock_until::Main();
    recursive_mutex::Main();
}