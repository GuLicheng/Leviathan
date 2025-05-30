#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <print>
#include <tuple>
#include <ranges>
#include <functional>
#include <map>
#include <random>
#include <algorithm>
#include <chrono>
#include <leviathan/algorithm/all.hpp>

template <typename Sorter, typename RandomAccessRange>
void benchmark_sort(Sorter sorter, RandomAccessRange range, const char* message)
{
    using clock = std::chrono::high_resolution_clock;

	auto tp1 = clock::now();
    sorter(range.begin(), range.end());
	auto tp2 = clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(tp2 - tp1).count();

    std::println("Sorting {} took {} milliseconds. Is sorted ? {}.", message, duration, std::ranges::is_sorted(range));
}

int main()
{
    std::vector<int> vec;
    std::random_device rd;

    for (int i = 0; i < 10000000; ++i)
    {
        vec.emplace_back(rd());
    }

    benchmark_sort(std::ranges::stable_sort, vec, "std_stable_sort");
    benchmark_sort(std::ranges::sort, vec, "std_sort");
    benchmark_sort(cpp::ranges::tim_sort, vec, "tim_sort");
}
