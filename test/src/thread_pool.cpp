#include <vector>
#include <iostream>
#include <lv_cpp/thread_pool.hpp>

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