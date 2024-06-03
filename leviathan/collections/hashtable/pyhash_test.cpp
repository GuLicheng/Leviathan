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

#include <unordered_set>

TEST_CASE("random test")
{
    std::unordered_set<int> s1;
    pyhashset s2;

    constexpr auto N = 100000;
    static std::random_device rd;

    // Insert
    for (int i = 0; i < N; ++i) 
    {
        auto x = rd();
        auto [it1, succeed1] = s1.insert(x);
        auto [it2, succeed2] = s2.insert(x);
    
        CHECK(succeed1 == succeed2);
    }

    CHECK(s1.size() == s2.size());

    // Search
    for (int i = 0; i < N; ++i) 
    {
        auto x = rd();
        auto succeed1 = s1.contains(x);
        auto succeed2 = s2.contains(x);
    
        CHECK(succeed1 == succeed2);
    }

    // Remove
    for (int i = 0; i < N; ++i) 
    {
        auto x = rd();
        auto k1 = s1.erase(x);
        auto k2 = s2.erase(x);
    
        CHECK(k1 == k2);
    }

    CHECK(s1.size() == s2.size());

}




