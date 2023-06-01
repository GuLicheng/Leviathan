#include <leviathan/collections/internal/common.hpp>
#include <catch2/catch_all.hpp>

struct MoveOnly
{
    int v;

    MoveOnly(int x) : v(x) { }

    MoveOnly(const MoveOnly&) = delete;

    MoveOnly(MoveOnly&& other)
        : v(std::exchange(other.v, -1)) 
    { }
};

TEST_CASE("value_handle")
{
    using allocator_type = std::allocator<MoveOnly>;
    allocator_type alloc;
    ::leviathan::collections::value_handle<MoveOnly, allocator_type> v1(alloc, 1);

    REQUIRE((*v1).v == 1);

    MoveOnly v2 = *v1;

    REQUIRE((*v1).v == -1);
    REQUIRE(v2.v == 1);
}











