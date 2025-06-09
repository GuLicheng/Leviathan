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
#include <ranges>
#include "PdqSort.hpp"
// #include <leviathan/powersort/src/sorts/powersort.h>

using ValueT = double;
using Iterator = std::vector<ValueT>::iterator;

// inline cpp::ranges::detail::sorter<algorithms::powersort<Iterator>> PowerSort;

int main(int argc, char* argv[])
{
    std::vector<ValueT> vec;
    std::random_device rd;

    int N = 10000000 / 2 * 2; // Default number of elements to sort

    if (argc > 1)
    {
        N = std::atoi(argv[1]);
    }

    for (int i = 0; i < N; ++i)
    {
        vec.emplace_back(rd());
    }

    // std::sort(vec.begin(), vec.end());

    std::vector<ValueT> all_zeros(N, 0);
    std::vector<ValueT> ascending(std::from_range, std::views::iota(0, N));
    std::vector<ValueT> descending(std::from_range, std::views::iota(0, N) | std::views::reverse);

    // std::ranges::reverse(ascending | std::views::stride(9));

    cpp::benchmark_sorter()
                    //    .add_sorter(std::ranges::stable_sort, "std::stable_sort")
                    //    .add_sorter(cpp::ranges::tim_sort, "tim_sort")
                    //    .add_sorter(cpp::ranges::power_sort, "power_sort")
                       .add_sorter(cpp::ranges::intro_sort, "intro_sort")
                       .add_sorter(std::ranges::sort, "std::sort")
                    //    .add_sorter(cpp::ranges::pdq_sort, "pdq_sort")
                       .add_sorter(cpp::ranges::pdq_sort_branchless, "pdq_sort_branchless")
                    //    .add_sorter(PdqSort, "pdqsort_official")
                       .add_sorter(PdqSortBranchless, "pdqsortbranchless_official")
                       (vec, "random distribution")
                    //    (ascending, "ascending distribution")
                    //    (descending, "descending distribution")
                    //    (all_zeros, "all zeros distribution")
                    ;

    std::println("Done sorting {} elements.", N);

}
