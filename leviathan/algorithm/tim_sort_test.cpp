#include "tim_sort.hpp"

#include <catch2/catch_all.hpp>
#include <utility>
#include <random>

TEST_CASE("TimSortSimpleTest")
{
    int arr[] = { 3, 2, 1 };

    cpp::ranges::tim_sort(arr);

    REQUIRE(arr[0] == 1);
    REQUIRE(arr[1] == 2);
    REQUIRE(arr[2] == 3);
}

TEST_CASE("stable")
{
    std::vector<std::pair<int, int>> vec;
    
    std::random_device rd;

    for (int i = 0; i < 10000; ++i)
    {
        vec.emplace_back(rd() % 10000, i);
    }

    auto vec1 = vec;

    cpp::ranges::tim_sort(vec, std::ranges::less(), &std::pair<int, int>::first);
    std::ranges::stable_sort(vec1, std::ranges::less(), &std::pair<int, int>::first);

    REQUIRE(vec == vec1);
}
