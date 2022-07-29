#define CATCH_CONFIG_MAIN

#include <lv_cpp/collections/internal/skip_list.hpp>
#include <thirdpart/catch.hpp>

using namespace leviathan::collections;
using SetT = skip_set<int>;

TEST_CASE("observer", "[empty][size]")
{
    SetT h;
    REQUIRE(h.size() == 0);
    REQUIRE(h.empty());
}

TEST_CASE("search elements", "[iterator][contains][find][lower_bound][upper_bound][equal_range][count]")
{
    SetT h;

    h.insert(1);
    h.insert(3);
    h.insert(5);
    h.insert(7);
    h.insert(9);

    // contains
    REQUIRE(h.contains(1));
    REQUIRE(!h.contains(0));

    // find
    REQUIRE(h.find(1) != h.end());
    REQUIRE(h.find(2) == h.end());
    REQUIRE(h.find(3) != h.end());

    // lower_bound
    REQUIRE(*h.lower_bound(1) == 1);
    REQUIRE(*h.lower_bound(2) == 3);
    REQUIRE(h.lower_bound(10) == h.end());

    // upper_bound
    REQUIRE(*h.upper_bound(1) == 3);
    REQUIRE(*h.upper_bound(2) == 3);
    REQUIRE(*h.upper_bound(3) == 5);
    REQUIRE(h.upper_bound(9) == h.end());
    REQUIRE(h.upper_bound(10) == h.end());

    // equal_range
    auto [a1, b1] = h.equal_range(0);
    REQUIRE(a1 == b1);
    auto [a2, b2] = h.equal_range(1);
    REQUIRE(a2 != b2);
    auto [a3, b3] = h.equal_range(10);
    REQUIRE(a3 == b3);

    // count
    REQUIRE(h.count(0) == 0);
    REQUIRE(h.count(1) == 1);

}


TEST_CASE("remove elements", "[iterator][remove][clear][find][size][empty]")
{
    SetT h;

    REQUIRE(h.erase(0) == 0);

    h.insert(1);
    h.insert(2);
    h.insert(3);

    REQUIRE(h.erase(1) == 1);
    REQUIRE(h.erase(1) == 0);
    REQUIRE(h.erase(2) == 1);
    REQUIRE(h.size() == 1);

    // { 0, 3 }
    h.insert(0);
    REQUIRE(h.size() == 2);

    h.erase(h.find(0));
    REQUIRE(h.size() == 1);

    h.erase(h.find(3));
    REQUIRE(h.empty());
}


