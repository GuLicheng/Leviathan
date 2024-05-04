// #include <mutex>
// #include <shared_mutex>
#include "mutex.hpp"
#include "lock.hpp"
#include <thread>
#include <list>
#include <iostream>
#include <vector>

#define READ_THREAD_COUNT 8  
#define LOOP_COUNT 5000000  

typedef SharedLock<SharedMutex> ReadLock;
typedef LockGuard<SharedMutex> WriteLock;
typedef LockGuard<Mutex> NormalLock;

class shared_mutex_counter {
public:
    shared_mutex_counter() = default;

    unsigned int get() const {
        ReadLock lock(mutex);
        return value;
    }

    void increment() {
        WriteLock lock(mutex);
        value++;
    }

private:
    mutable SharedMutex mutex;
    unsigned int value = 0;
};

class mutex_counter {
public:
    mutex_counter() = default;

    unsigned int get() const {
        NormalLock lock(mutex);
        return value;
    }

    void increment() {
        NormalLock lock(mutex);
        value++;
    }

private:
    mutable Mutex mutex;
    unsigned int value = 0;
};

class timers
{
public:
    timers()
    {
        m_begin = std::chrono::high_resolution_clock::now();
    }

    ~timers()
    {
        m_end = std::chrono::high_resolution_clock::now();
        Consuming();
    }

    void Consuming()
    {
        std::cout << "Time-consuming:" << std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(m_end - m_begin).count() << std::endl;
    }

private:
    std::chrono::high_resolution_clock::time_point m_begin;
    std::chrono::high_resolution_clock::time_point m_end;
};


void test_shared_mutex()
{
    shared_mutex_counter counter;
    unsigned int temp;

    auto writer = [&counter]() {
        for (unsigned int i = 0; i < LOOP_COUNT; i++){
            counter.increment();
        }
    };

    auto reader = [&counter, &temp]() {
        for (unsigned int i = 0; i < LOOP_COUNT; i++) {
            temp = counter.get();
        }
    };

    std::cout << "----- shared mutex test ------" << std::endl;
    std::list<std::shared_ptr<std::thread>> threadlist;
    {
        timers timer;

        for (int i = 0; i < READ_THREAD_COUNT; i++)
        {
            threadlist.push_back(std::make_shared<std::thread>(reader));
        }
        std::shared_ptr<std::thread> pw = std::make_shared<std::thread>(writer);

        for (auto &it : threadlist)
        {
            it->join();
        }
        pw->join();
    }
    std::cout <<"count:"<< counter.get() << ", temp:" << temp << std::endl;
}

void test_mutex()
{
    mutex_counter counter;
    unsigned int temp;

    auto writer = [&counter]() {
        for (unsigned int i = 0; i < LOOP_COUNT; i++) {
            counter.increment();
        }
    };

    auto reader = [&counter, &temp]() {
        for (unsigned int i = 0; i < LOOP_COUNT; i++) {
            temp = counter.get();
        }
    };

    std::cout << "----- mutex test ------" << std::endl;
    std::list<std::shared_ptr<std::thread>> threadlist;
    {
        timers timer;

        for (int i = 0; i < READ_THREAD_COUNT; i++)
        {
            threadlist.push_back(std::make_shared<std::thread>(reader));
        }

        std::shared_ptr<std::thread> pw = std::make_shared<std::thread>(writer);

        for (auto &it : threadlist)
        {
            it->join();
        }
        pw->join();
    }
    std::cout << "count:" << counter.get() << ", temp:" << temp << std::endl;
}



int main()
{
    test_shared_mutex();
    test_mutex();
    return 0;
}

// //output
// ----- shared mutex test ------
// Time-consuming:4309.85
// count:5000000, temp:5000000
// ----- mutex test ------
// Time-consuming:10541.4
// count:5000000, temp:4953412