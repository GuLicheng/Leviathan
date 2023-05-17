#if 0
#include <iostream>
#include <functional>
#include <lv_cpp/memory_invoke.hpp>
#include <lv_cpp/utils/timer.hpp>

namespace lv = leviathan;

std::function<uint64_t(uint64_t)> fib1 = 
lv::memoized_invoke<lv::mode::fifo>(
    [f=&fib1](uint64_t n) -> uint64_t { return n < 2 ? n : fib1(n - 1) + fib1(n - 2); });
// use [f = &fib] or [f = fib] to replace [&] to avoid warning, 
// but it will still repro warning in clang 

/*
auto fib2 = [](auto&& self, uint64_t n) -> uint64_t 
{
    return n < 2 ? n : self(self, n - 1) + self(self, n - 2);
};
// auto cache = lv::memoized_invoke<lv::mode::map>(fib2);  
// our traits cannot adapt template function such as auto&& 
*/

std::function<uint64_t(uint64_t)> fib2 = 
lv::memoized_invoke<lv::mode::map>(
    [f=&fib2](uint64_t n) -> uint64_t { return n < 2 ? n : fib2(n - 1) + fib2(n - 2); });

std::function<uint64_t(uint64_t)> fib3 = 
lv::memoized_invoke<lv::mode::hash>(
    [f=&fib3](uint64_t n) -> uint64_t { return n < 2 ? n : fib3(n - 1) + fib3(n - 2); });


auto test1(int n)
{
    // list
    lv::timer t;
    return fib1(n);
}

auto test2(int n)
{
    // map
    lv::timer t;
    return fib2(n);
}

auto test3(int n)
{
    // map
    lv::timer t;
    return fib3(n);
}

int main()
{
    int n;
    std::cin >> n;
    // stack memory < 1M !!!
    std::cout << "test1 for list\n";
    std::cout << test1(n) << std::endl;
    std::cout << "test1 for map\n";
    std::cout << test2(n) << std::endl;
    std::cout << "test3 for hash\n";
    std::cout << test3(n) << std::endl;

    return 0;
}

#endif

int main()
{
    /* code */
    return 0;
}
