#include <lv_cpp/collections/internal/skip_list.hpp>

#include "random_range.hpp"

#include <catch2/catch_all.hpp>

constexpr int N = 10'000;

auto range_generator = leviathan::random_range(N, N * 10);

inline std::vector<int> numbers = range_generator.random_range_int();
inline constexpr auto default_num = N; 

template <typename OrderedSet>
auto random_insert_test()
{
    OrderedSet s;
    for (auto val : numbers) s.insert(val);
    assert(s.size() <= default_num);
    return s.size();
}

TEST_CASE("duplicate_ordered_collections_random_insert skip_list vs std::map")
{
    using SkipList = leviathan::collections::skip_set<int>;

    BENCHMARK("skip_list")
    {
        return random_insert_test<SkipList>();
    };

    BENCHMARK("std::set")
    {
        return random_insert_test<std::set<int>>();
    };
}


