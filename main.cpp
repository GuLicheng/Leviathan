#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <print>
#include <tuple>
#include <ranges>

int main()
{
    std::vector<int> v(21, 1);
    std::println("Hello, World!{}", std::make_tuple(1, 2, 3)); // C++20 feature
}
