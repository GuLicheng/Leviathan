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

struct STLHeapSorter
{
    template <typename I, typename Comp>
    static constexpr void operator()(I first, I last, Comp comp)
    {
        std::ranges::make_heap(first, last, std::ref(comp));
        std::ranges::sort_heap(first, last, std::move(comp));
    }
};

inline constexpr cpp::ranges::detail::sorter<STLHeapSorter> STLHeapSort;

std::ranges::less l1;
std::identity id1;

auto cp = cpp::ranges::detail::make_comp_proj(l1, id1);

// static_assert(std::is_empty_v<decltype(l1)>);
// static_assert(std::is_empty_v<decltype(id1)>);
// static_assert(std::is_empty_v<decltype(cp)>);


int main(int argc, char* argv[])
{
    std::vector<ValueT> vec;
    std::random_device rd;

    int N = 10000000 / 2 ; // Default number of elements to sort

    if (argc > 1)
    {
        N = std::atoi(argv[1]);
    }

    for (int i = 0; i < N; ++i)
    {
        vec.emplace_back(i);
    }

    std::ranges::reverse(vec | std::views::stride(9));

    cpp::benchmark_sorter()
                       .add_sorter(cpp::ranges::intro_sort, "intro_sort")
                       .add_sorter(std::ranges::sort, "std::sort")
                       .add_sorter(STLHeapSort, "stl_heap_sort")
                        .add_sorter(cpp::ranges::pdq_sort_branchless, "pdq_sort_branchless")
                        .add_sorter(cpp::ranges::pdq_sort, "pdq_sort")
                        .add_sorter(PdqSortBranchless, "pdqsortbranchless_official")
                       .random()
                       .random_string()
                    //    .all_zeros()
                    //    .pipeline()
                    //    .ascending()
                    //    .descending()
                        ;

}
