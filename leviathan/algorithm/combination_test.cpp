// https://leetcode.cn/problems/subsets-ii/

#include "combination.hpp"

#include <catch2/catch_all.hpp>
#include <vector>
#include <list>

TEST_CASE("combination random access iterator")
{
    std::vector<int> vec = { 1, 2, 2 };

    std::vector<int> result[2] = {
        std::vector<int>{ 1, 2, 2 },
        std::vector<int>{ 2, 2, 1 },
    };

    int i = 0;

    do {
        REQUIRE(vec == result[i++]);
    } while (cpp::ranges::next_combination(vec.begin(), vec.begin() + 2, vec.end()).found);
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
    } while (cpp::ranges::next_combination(vec, vec.begin() + 2).found);
}

TEST_CASE("combination bidirectional iterator")
{
    std::list<int> vec = { 1, 2, 2 };

    std::list<int> result[2] = {
        std::list<int>{ 1, 2, 2 },
        std::list<int>{ 2, 2, 1 },
    };

    int i = 0;

    do {
        REQUIRE(vec == result[i++]);
    } while (cpp::ranges::next_combination(vec.begin(), std::next(vec.begin(), 2), vec.end()).found);
}
