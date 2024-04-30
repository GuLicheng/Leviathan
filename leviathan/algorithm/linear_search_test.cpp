#include "linear_search.hpp"
#include <catch2/catch_all.hpp>
#include <vector>

TEST_CASE("LinearSearchWithNonEmptyArray")
{
    int arr[] = { 1, 2, 3 };

    REQUIRE(leviathan::algorithm::linear_search(arr, arr + 3, 1) == true);
    REQUIRE(leviathan::algorithm::linear_search(arr, arr + 3, 2) == true);
    REQUIRE(leviathan::algorithm::linear_search(arr, arr + 3, 3) == true);
    REQUIRE(leviathan::algorithm::linear_search(arr, arr + 3, 0) == false);
}

TEST_CASE("LinearSearchRangeWithNonEmptyArray")
{
    int arr[] = { 1, 2, 3 };

    REQUIRE(leviathan::algorithm::ranges::linear_search(arr, 1) == true);
    REQUIRE(leviathan::algorithm::ranges::linear_search(arr, 2) == true);
    REQUIRE(leviathan::algorithm::ranges::linear_search(arr, 3) == true);
    REQUIRE(leviathan::algorithm::ranges::linear_search(arr, 0) == false);
}

TEST_CASE("LinearSearchRangeWithEmptyArray")
{
    std::vector<int> v;

    REQUIRE(leviathan::algorithm::ranges::linear_search(v, 0) == false);
}
