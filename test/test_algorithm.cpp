#define CATCH_CONFIG_MAIN

#include <thirdpart/catch.hpp>
#include <lv_cpp/algorithm/core.hpp>

#include <vector>

using namespace leviathan;

TEST_CASE("pair_wise_stride_transform")
{
    std::vector<int> v = { 0, 1, 2, 3, 4 }, dest;
    auto [ret, _] = pair_wise_stride_transform(v.begin(), v.end(), std::back_inserter(dest), std::plus<>());
    REQUIRE(std::ranges::equal(dest, std::vector<int>{ 1, 5 }));
    REQUIRE(std::distance(ret, v.end()) == 1);
    REQUIRE(*ret == 4);
}

TEST_CASE("pair_wise_transform")
{
    std::vector<int> v = { 0, 1, 2, 3, 4 }, dest;
    auto [ret, _] = pair_wise_transform(v.begin(), v.end(), std::back_inserter(dest), std::plus<>());
    REQUIRE(dest == std::vector<int>{ 1, 3, 5, 7 });
}

TEST_CASE("next_combination")
{
    std::vector values = { 1, 2, 3 };
    std::vector<std::vector<int>> result = {
        std::vector<int> { 1, 2, 3 },
        std::vector<int> { 1, 3, 2 },
        std::vector<int> { 2, 3, 1 }
    };
    int i = 0;

    do 
    {
        REQUIRE(std::ranges::equal(values, result[i++]));
    } while (leviathan::next_combination(values, values.begin() + 2).found);

    values = { 1, 2, 2 };
    result = {
        std::vector<int> { 1, 2, 2 },
        std::vector<int> { 2, 2, 1 }
    };
    i = 0;
    do 
    {
        REQUIRE(std::ranges::equal(values, result[i++]));
    } while (leviathan::next_combination(values, values.begin() + 2).found);
    
}

