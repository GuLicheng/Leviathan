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
#include <leviathan/algorithm/benchmark_sorter.hpp>
#include <leviathan/powersort/src/sorts/powersort.h>
#include <leviathan/powersort/src/sorts/peeksort.h>

using ValueT = double;
using Iterator = std::vector<ValueT>::iterator;

inline cpp::ranges::detail::sorter<algorithms::powersort<Iterator>> PowerSort;

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

    cpp::benchmark_sorter()
                       .add_sorter(std::ranges::stable_sort, "std::stable_sort")
                       .add_sorter(cpp::ranges::tim_sort, "tim_sort")
                       .add_sorter(cpp::ranges::power_sort, "power_sort")
                       .add_sorter(PowerSort, "PowerSort")
                       .add_sorter(std::ranges::sort, "std::sort")
                       (vec);

    std::println("Done sorting {} elements.", N);
}
