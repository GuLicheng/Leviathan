#include <leviathan/algorithm/heap.hpp>
#include <vector>
#include <catch2/catch_all.hpp>
#include "random_range.hpp"

using BinaryHeapFunction = cpp::ranges::nd_heap_fn<2>;

void STLHeapSort(auto& vec)
{
    std::ranges::make_heap(vec);
    std::ranges::sort_heap(vec);
}

void HeapSort(auto& vec)
{
    // std::ranges::make_heap(vec);
    BinaryHeapFunction::make_heap(vec);
    BinaryHeapFunction::sort_heap(vec);
}

inline constexpr auto default_num = 100000;

inline auto random_generator = cpp::random_range(default_num, default_num * 10);

inline auto random_int = random_generator.random_range_int(); 

TEST_CASE("make heap")
{
    BENCHMARK("leviathan make heap")
    {
        std::vector<int> vec = random_int;
        HeapSort(vec);
        CHECK(std::is_sorted(vec.begin(), vec.end()));
    };

    BENCHMARK("stl")
    {
        std::vector<int> vec = random_int;
        STLHeapSort(vec);
        CHECK(std::is_sorted(vec.begin(), vec.end()));
    };
}
