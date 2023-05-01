// https://leetcode.cn/problems/subsets-ii/

#include <catch2/catch_all.hpp>
#include <lv_cpp/algorithm/combination_range.hpp>
#include <vector>

TEST_CASE("combination")
{
    std::vector<int> vec = { 1, 2, 2 };

    std::vector<int> result[2] = {
        std::vector<int>{ 1, 2, 2 },
        std::vector<int>{ 2, 2, 1 },
    };

    int i = 0;

    do {
        REQUIRE(vec == result[i++]);
    } while (leviathan::next_combination(vec.begin(), vec.begin() + 2, vec.end()));
}

TEST_CASE("range_combination")
{
    std::vector<int> vec = { 1, 2, 3 };

    std::vector<int> result[3] = {
        std::vector<int>{ 1, 2, 3 },
        std::vector<int>{ 1, 3, 2 },
        std::vector<int>{ 2, 3, 1 },
    };

    int i = 0;

    do {
        REQUIRE(vec == result[i++]);
    } while (leviathan::ranges::next_combination(vec, vec.begin() + 2).found);
}
