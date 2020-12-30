#include <iostream>
#include <functional>
#include <my_cpp/memory_invoke.hpp>
#include <my_cpp/timer.hpp>

namespace lv = leviathan;

std::function<uint64_t(uint64_t)> fib1 = 
lv::memoized_invoke<lv::mode::fifo>([f=&fib1](uint64_t n) -> uint64_t { return n < 2 ? n : fib1(n - 1) + fib1(n - 2); });
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
lv::memoized_invoke<lv::mode::map>([f=&fib2](uint64_t n) -> uint64_t { return n < 2 ? n : fib2(n - 1) + fib2(n - 2); });

void test1(int n)
{
    // list
    lv::timer t;
    std::cout << fib1(n) << std::endl;
}

void test2(int n)
{
    // map
    lv::timer t;
    std::cout << fib2(n) << std::endl;
}

int main()
{

    // stack memory < 1M !!!
    test2(1000);
    test2(1000);

    return 0;
}

