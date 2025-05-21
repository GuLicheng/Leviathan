#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <print>
#include <tuple>
#include <ranges>
#include <functional>
#include <leviathan/print.hpp>
#include <leviathan/meta/type.hpp>
#include <leviathan/meta/function_traits.hpp>
#include <leviathan/algorithm/combination.hpp>
#include <map>
#include <random>
#include <leviathan/stopwatch.hpp>
#include <algorithm>
#include <leviathan/algorithm/tim_sort.hpp>

int Test1()
{
    std::vector<int> vec;
    int i = 0;
    vec.insert_range(vec.end(), std::views::iota(0, 30));

    do { ++i; } 
    while (cpp::ranges::detail::combination_impl(vec.begin(), vec.begin() + 15, vec.end(), std::less<>()));
    return i;
}

int main()
{
    cpp::time::stopwatch sw;

    sw.restart();
    auto c1 = Test1();
    sw.stop();

    std::cout << "Test1: " << sw.elapsed_milliseconds() << "ms" << std::endl;

    std::println("Test1: {}", c1);
}
