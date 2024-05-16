#include "associative_container_interface.hpp"
#include <catch2/catch_all.hpp>
#include <set>
#include <map>
#include <array>
#include <algorithm>
#include <ranges>

using namespace leviathan::collections;

template <typename T> 
class AssociativeContainerSet : 
    public associative_container_lookup_interface<true>,
    public associative_container_insertion_interface
{
    using base = std::set<T>;

    base m_s;

public:

    inline static double SearchedValue = 1.0;

    void TestLookupMethods() 
    {
        auto it = find(SearchedValue); // This will call lower_bound
    }

public:

    AssociativeContainerSet() = default;

    using value_type = T;
    using iterator = typename base::iterator;
    using const_iterator = typename base::const_iterator;
    using reverse_iterator = typename base::reverse_iterator;
    using const_reverse_iterator = typename base::const_reverse_iterator;

    using key_compare = typename base::key_compare;
    using key_type = typename base::key_type;
    using key_value = identity<T>;

    auto key_comp() const { return m_s.key_comp(); }

    auto begin() { return m_s.begin(); }
    auto begin() const { return m_s.begin(); }

    auto end() { return m_s.end(); }
    auto end() const { return m_s.end(); }

    // Lookup interface core APIs
    template <typename U>
    auto lower_bound(const U& x) 
    { 
        return m_s.lower_bound(x); 
    }

    template <typename U>
    auto lower_bound(const U& x) const 
    { 
        return m_s.lower_bound(x); 
    }

    // Insertion interface core APIs
    template <typename... Args>
    auto emplace(Args&&... args)
    {
        return m_s.emplace((Args&&) args...); 
    }

    template <typename... Args>
    auto emplace_hint(const iterator& pos, Args&&... args)
    {
        return m_s.emplace_hint(pos, (Args&&) args...);
    }

};

TEST_CASE("associative_container_lookup_interface")
{
    AssociativeContainerSet<int> s;

    auto ZeroIter = s.emplace(0).first;
    auto TwoIter = s.emplace(2).first;
    auto Sentinel = s.end();

    // Search [-1, 0, 1, 2, 3]

    std::array SearchedValues = { -1, 0, 1, 2, 3 };

    SECTION("find")
    {
        std::array FindResults = { Sentinel, ZeroIter, Sentinel, TwoIter, Sentinel };
        auto result = std::ranges::equal(SearchedValues, FindResults, [&](auto v, auto it) 
        {
            return s.find(v) == it;
        });
        CHECK(result);
    }

    SECTION("count")
    {
        std::array CountResults = { 0, 1, 0, 1, 0 };
        auto result = std::ranges::equal(SearchedValues, CountResults, [&](auto v, size_t c) 
        {
            return s.count(v) == c;
        });
        CHECK(result);
    }

    SECTION("equal_range")
    {
        std::array LowerBoundResults = { ZeroIter, ZeroIter, TwoIter, TwoIter, Sentinel };
        std::array UpperBoundResults = { ZeroIter, TwoIter, TwoIter, Sentinel, Sentinel };

        auto result = std::ranges::equal(
            SearchedValues,
            std::views::zip(LowerBoundResults, UpperBoundResults),
            [&](auto v, auto pair)
            {
                return s.equal_range(v) == pair;
            }
        );
        CHECK(result);
    }

    SECTION("upper_bound")
    {
        std::array UpperBoundResults = { ZeroIter, TwoIter, TwoIter, Sentinel, Sentinel };
        auto result = std::ranges::equal(SearchedValues, UpperBoundResults, [&](auto v, auto it) 
        {
            return s.upper_bound(v) == it;
        });
        CHECK(result);
    }

    SECTION("contains")
    {
        std::array ContainsResults = { 0, 1, 0, 1, 0 };
        auto result = std::ranges::equal(SearchedValues, ContainsResults, [&](auto v, bool c) 
        {
            return s.contains(v) == !c;
        });
        CHECK(result);
    }

}


TEST_CASE("associative_container_insertion_interface")
{
    AssociativeContainerSet<int> s;

    std::array Values = { 0, 1, 2 };

    int x = 0;

    s.insert(x); // (1)

    s.insert(1); // (2)

    // s.insert(s.begin(), x); // (3) ?

    // s.insert(s.begin(), 2); // (4) ?

    s.insert(Values.begin(), Values.end()); // (5)

    s.insert({ 2, 3 }); // (6)

    std::array Results = { 0, 1, 2, 3 };

    auto result = std::ranges::equal(
        Results.begin(), Results.end(),
        s.begin(), s.end()
    );

    CHECK(result);
}











