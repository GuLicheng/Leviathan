#include <iostream>
#include "skiplist.hpp"
#include <catch2/catch_all.hpp>

using namespace leviathan::collections;

using SetT = skiplist<identity<int>, std::ranges::less, std::allocator<int>, true>;

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
    CHECK(h.contains(1));
    CHECK(!h.contains(0));

    // find
    CHECK(h.find(1) != h.end());
    CHECK(h.find(2) == h.end());
    CHECK(h.find(3) != h.end());

    // lower_bound
    CHECK(*h.lower_bound(1) == 1);
    CHECK(*h.lower_bound(2) == 3);
    CHECK(h.lower_bound(10) == h.end());

    // upper_bound
    CHECK(*h.upper_bound(1) == 3);
    CHECK(*h.upper_bound(2) == 3);
    CHECK(*h.upper_bound(3) == 5);
    CHECK(h.upper_bound(9) == h.end());
    CHECK(h.upper_bound(10) == h.end());

    // equal_range
    auto [a1, b1] = h.equal_range(0);
    CHECK(a1 == b1);
    auto [a2, b2] = h.equal_range(1);
    CHECK(a2 != b2);
    auto [a3, b3] = h.equal_range(10);
    CHECK(a3 == b3);

    // count
    CHECK(h.count(0) == 0);
    CHECK(h.count(1) == 1);

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
