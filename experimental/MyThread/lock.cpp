#include <iostream>
#include <chrono>
#include <thread>

#include "mutex.hpp"
#include "lock.hpp"

namespace DeadLock
{
    Mutex mt1;
    Mutex mt2;
    void deadLock(Mutex &mtA, Mutex &mtB)
    {
        // for thread1, mtA is mt1, for thread2, mtA is mt2 -> deadlock
        LockGuard<Mutex> lock1(mtA);
        std::cout << "get the first mutex"
                  << " in thread " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        LockGuard<Mutex> lock2(mtB);
        std::cout << "get the second mutex"
                  << " in thread " << std::this_thread::get_id() << std::endl;

        std::cout << "do something in thread " << std::this_thread::get_id() << std::endl;
    }

    void Main()
    {
        std::thread t1([&]
                       { deadLock(mt1, mt2); });
        std::thread t2([&]
                       { deadLock(mt2, mt1); });
        t1.join();
        t2.join();
    }
}

namespace NoDeadLock
{
    Mutex mt1;
    Mutex mt2;
    void deadLock(Mutex &mtA, Mutex &mtB)
    {
        Lock(mtA, mtB); // lock mtA and mtB at once
        LockGuard<Mutex> lock1(mtA, adopt_lock);
        std::cout << "get the first mutex"
                  << " in thread " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        LockGuard<Mutex> lock2(mtB, adopt_lock);
        std::cout << "get the second mutex"
                  << " in thread " << std::this_thread::get_id() << std::endl;

        std::cout << "do something in thread " << std::this_thread::get_id() << std::endl;
    }

    void Main()
    {
        std::thread t1([&]
                       { deadLock(mt1, mt2); });
        std::thread t2([&]
                       { deadLock(mt2, mt1); });
        t1.join();
        t2.join();
    }
}

namespace TryLockTest
{

    int main()
    {
        int foo_count = 0;
        Mutex foo_count_mutex;
        int bar_count = 0;
        Mutex bar_count_mutex;
        int overall_count = 0;
        bool done = false;
        Mutex done_mutex;

        auto increment = [](int &counter, Mutex &m, const char *desc)
        {
            for (int i = 0; i < 10; ++i)
            {
                UniqueLock<Mutex> lock(m);
                ++counter;
                std::cout << desc << ": " << counter << '\n';
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        };

        std::thread increment_foo(increment, std::ref(foo_count),
                                  std::ref(foo_count_mutex), "foo");
        std::thread increment_bar(increment, std::ref(bar_count),
                                  std::ref(bar_count_mutex), "bar");

        std::thread update_overall([&]()
                                   {
                                       done_mutex.lock();
                                       while (!done)
                                       {
                                           done_mutex.unlock();
                                           int result = ::TryLock(foo_count_mutex, bar_count_mutex);
                                           if (result == -1)
                                           {
                                               overall_count += foo_count + bar_count;
                                               foo_count = 0;
                                               bar_count = 0;
                                               std::cout << "overall: " << overall_count << '\n';
                                               foo_count_mutex.unlock();
                                               bar_count_mutex.unlock();
                                           }
                                           std::this_thread::sleep_for(std::chrono::seconds(2));
                                           done_mutex.lock();
                                       }
                                       done_mutex.unlock();
                                   });

        increment_foo.join();
        increment_bar.join();
        done_mutex.lock();
        done = true;
        done_mutex.unlock();
        update_overall.join();

        std::cout << "Done processing\n"
                  << "foo: " << foo_count << '\n'
                  << "bar: " << bar_count << '\n'
                  << "overall: " << overall_count << '\n';
        return 0;
    }
}
namespace NoDeadLock2
{
    Mutex mt1;
    Mutex mt2;
    void deadLock(Mutex &mtA, Mutex &mtB)
    {
        ScopedLock(mtA, mtB);
        std::cout << "get the first mutex"
                  << " in thread " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        LockGuard<Mutex> lock2(mtB, adopt_lock);
        std::cout << "get the second mutex"
                  << " in thread " << std::this_thread::get_id() << std::endl;

        std::cout << "do something in thread " << std::this_thread::get_id() << std::endl;
    }

    void Main()
    {
        std::thread t1([&]
                       { deadLock(mt1, mt2); });
        std::thread t2([&]
                       { deadLock(mt2, mt1); });
        t1.join();
        t2.join();
    }
}

int main()
{
    NoDeadLock::Main();
    std::cout << "==============================================\n";
    NoDeadLock2::Main();
    std::cout << "==============================================\n";
    TryLockTest::main();
    std::cout << "==============================================\n";
    DeadLock::Main();
}