// #include <generator.h>
#include <lv_cpp/generator.h>
#include <lv_cpp/myranges.h>
#include <lv_cpp/myprint.h>
#include <random>
#include <iostream>
#include <ranges>

namespace vw = std::views;

auto coro() {
    auto gen = [](int n = 100) ->::std::generator<int> {
        auto gen = std::mt19937 (std::random_device{}());
        auto d = std::uniform_int_distribution (-100,100);
        for (auto _ :vw::iota(0, n)) {
            co_yield d(gen);
        }
    };
    auto v = gen()
            | vw::take(54)
            | vw::enumerate
            | vw::print
            ;
    std::cout << v;
}
// g++ -std=c++20 -fcoroutines .\test\src\generator_test.cpp  -o a
int main()
{
    coro();
}