#include "pyhash.hpp"
#include <catch2/catch_all.hpp>

using pyhashset = leviathan::collections::py_hashtable<
    leviathan::collections::identity<int>,
    std::hash<int>,
    std::equal_to<int>,
    std::allocator<int>
>;

TEST_CASE("hash table insert")
{
    pyhashset h;

    h.emplace(1);
    h.emplace(2);
    h.emplace(1);
    h.emplace(1);
    h.insert(1);

    CHECK(h.size() == 2);

}

