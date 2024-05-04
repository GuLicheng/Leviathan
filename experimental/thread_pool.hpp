#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__

#include <mutex>
#include <queue>
#include <future>
#include <memory>
#include <thread>
#include <vector>
#include <functional>
#include <type_traits>
#include <condition_variable>

namespace leviathan
{

class thread_pool
{
public:
    thread_pool(size_t size);

    template <typename F, typename... Args>
    auto post(F &&f, Args &&...args);

    ~thread_pool();

private:
    bool stop = false;
    std::mutex mutex;
    std::condition_variable cond;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
};

thread_pool::thread_pool(size_t size)
{
    for (size_t i = 0; i != size; ++i)
        workers.emplace_back([this]
                                {
            while (true)
            {
                std::function<void()> task;
                {
                    std::unique_lock lock(mutex);
                    cond.wait(lock, [this]{ return stop || !tasks.empty(); });
                    if (stop && tasks.empty())
                        return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            } });
}

template <typename F, typename... Args>
auto thread_pool::post(F &&f, Args &&...args)
{
    using task_type = std::packaged_task<std::invoke_result_t<F, Args...>()>;
    auto task = std::make_shared<task_type>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto fut = task->get_future();
    {
        std::unique_lock lock(mutex);
        tasks.emplace([task]
                        { (*task)(); });
    }
    cond.notify_one();
    return fut;
}

thread_pool::~thread_pool()
{
    {
        std::unique_lock lock(mutex);
        stop = true;
    }
    cond.notify_all();
    for (auto &worker : workers)
        worker.join();s
}

} // namespace leviathan

#endif


/*
#include <vector>
#include <iostream>
#include <leviathan/thread_pool.hpp>

using namespace leviathan;
// g++ -std=c++20 -s -Wall -Wextra -pthread .\test\src\thread_pool.cpp -o a 
int main()
{
    thread_pool pools(4);
    std::vector<std::future<int>> results;

    for (int i = 0; i != 16; ++i)
    {
        results.emplace_back
        (
            pools.post([i]
            {
                return 2 * i + 1;
            })
        );
    }

    for (auto&& result : results)
        std::cout << result.get() << " ";
    std::cout << std::endl;

    return 0;
}
*/