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
#include <leviathan/powersort/src/sorts/powersort.h>
#include <leviathan/powersort/src/sorts/peeksort.h>

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

using ValueT = int;
using Iterator = std::vector<ValueT>::iterator;

inline cpp::ranges::detail::sorter<algorithms::powersort<Iterator>> PowerSort;
inline cpp::ranges::detail::sorter<algorithms::peeksort<Iterator>> PeekSort;

int main(int argc, char* argv[])
{
    std::vector<ValueT> vec;
    std::random_device rd;

    int N = 10000000; // Default number of elements to sort

    if (argc > 1)
    {
        N = std::atoi(argv[1]);
    }

    for (int i = 0; i < N; ++i)
    {
        vec.emplace_back(rd());
    }

    benchmark_sort(std::ranges::stable_sort, vec, "std::stable_sort");
    benchmark_sort(cpp::ranges::tim_sort, vec, "tim_sort");
    benchmark_sort(cpp::ranges::power_sort, vec, "power_sort");
    benchmark_sort(PeekSort, vec, "PeekSort");
    benchmark_sort(PowerSort, vec, "PowerSort");
    benchmark_sort(std::ranges::sort, vec, "std::sort");


    std::println("Done sorting {} elements.", N);
}
