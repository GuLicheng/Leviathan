#include <lv_cpp/collections/internal/sorted_list.hpp>
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

TEST_CASE("sorted_list_insert")
{

}

#include "test_random_int.hpp"

TEST_CASE("data structure is correct", "[insert][contains][erase]")
{
    using SortedListT = ::leviathan::collections::sorted_set<int>;
    ::leviathan::test::test_set_is_correct<SortedListT, true>();
}
