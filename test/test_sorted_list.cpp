#include <leviathan/collections/internal/sorted_list.hpp>
#include <catch2/catch_all.hpp>

template <typename T, size_t TruckSize>
using sorted_list_with_truck = leviathan::collections::sorted_list<
    std::tuple<T>,
    std::less<T>,
    std::allocator<T>,
    leviathan::collections::identity,
    true,
    TruckSize
>;

#include "leviathan/struct.hpp"

TEST_CASE("elements destroy")
{
    using T = Int32<>;
    {
        sorted_list_with_truck<T, 2> sl;
        sl.emplace(0);
        sl.emplace(1);
        sl.emplace(2);
        sl.emplace(3);
        sl.emplace(4);
        sl.emplace(5);
        sl.emplace(6);
        sl.emplace(7);
    }
    REQUIRE(T::total_construct() == T::total_destruct());
}

TEST_CASE("sorted_list_map")
{
    leviathan::collections::sorted_map<int, std::string> smap;
    smap.emplace(0, "Hello");
    smap.emplace(1, "World");
    smap.emplace(2, "!");

    REQUIRE(smap.find(0)->second == "Hello");
    REQUIRE(smap.find(1)->second == "World");
    REQUIRE(smap.find(2)->second == "!");
}

#include "test_random_int.hpp"

TEST_CASE("data structure is correct", "[insert][contains][erase]")
{
    using SortedListT = ::leviathan::collections::sorted_set<int>;
    ::leviathan::test::test_set_is_correct<SortedListT, true, leviathan::test::test_mode::All>();
}


