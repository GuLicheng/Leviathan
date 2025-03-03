#include "tim_sort.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("TimSortSimpleTest")
{
    int arr[] = { 3, 2, 1 };

    leviathan::algorithm::tim_sort(arr);

    REQUIRE(arr[0] == 1);
    REQUIRE(arr[1] == 2);
    REQUIRE(arr[2] == 3);
}
