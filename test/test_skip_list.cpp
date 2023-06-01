#define CATCH_CONFIG_MAIN

#include <leviathan/collections/internal/skip_list.hpp>
#include <catch2/catch_all.hpp>

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

#include "test_random_int.hpp"

TEST_CASE("data structure is correct", "[insert][contains][erase]")
{
    ::leviathan::test::test_set_is_correct<SetT, true>();
}

// #include "struct.hpp"
// #include "except_allocator.hpp"
// #include "fancy_ptr.hpp"

#include <leviathan/struct.hpp>
#include <leviathan/fancy_ptr.hpp>
#include <leviathan/record_allocator.hpp>

TEST_CASE("fancy pointer")
{
    ::leviathan::collections::skip_set<int, std::less<>, TrivialAllocator<int>> sl;
    sl.insert(0);
}

TEST_CASE("element destroy", "[dtor]")
{

    {
        ::leviathan::collections::skip_set<Int32<>> h1, h2, h3;

        for (int i = 0; i < 10; ++i)
        {
            h1.insert(Int32<>(i));
        }
    }

    auto a = Int32<>::total_construct();
    auto b = Int32<>::total_destruct();

    REQUIRE(a == b);
}

TEST_CASE("memory", "[dtor]")
{
    {
        ::leviathan::collections::skip_set<int, std::less<>, RecordAllocator<int>> h1, h2;

        for (int i = 0; i < 10; ++i)
        {
            h1.insert(i);
            // h1.emplace((long long)i);
        }
    }
    REQUIRE(CheckMemoryAlloc());
}

