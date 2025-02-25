#include "linear_search.hpp"
#include <catch2/catch_all.hpp>
#include <vector>

TEST_CASE("LinearSearch_WithNonEmptyArray")
{
    int arr[] = { 1, 2, 3 };

    REQUIRE(leviathan::algorithm::linear_search(arr, arr + 3, 1) == true);
    REQUIRE(leviathan::algorithm::linear_search(arr, arr + 3, 2) == true);
    REQUIRE(leviathan::algorithm::linear_search(arr, arr + 3, 3) == true);
    REQUIRE(leviathan::algorithm::linear_search(arr, arr + 3, 0) == false);
}

TEST_CASE("LinearSearchRange_WithNonEmptyArray")
{
    int arr[] = { 1, 2, 3 };

    REQUIRE(leviathan::algorithm::linear_search(arr, 1) == true);
    REQUIRE(leviathan::algorithm::linear_search(arr, 2) == true);
    REQUIRE(leviathan::algorithm::linear_search(arr, 3) == true);
    REQUIRE(leviathan::algorithm::linear_search(arr, 0) == false);
}

TEST_CASE("LinearSearchRange_WithEmptyArray")
{
    std::vector<int> v;

    REQUIRE(leviathan::algorithm::linear_search(v, 0) == false);
}

#include <utility>

TEST_CASE("LinearSearch_Enabling_List-initialization")
{
    using Point = std::pair<int, int>;

    std::vector<Point> v;

    v.emplace_back(0, 0);

    REQUIRE(leviathan::algorithm::linear_search(v.begin(), v.end(), { 0, 0 }) == true);
    REQUIRE(leviathan::algorithm::linear_search(v, { 1, 1 }) == false);
}
