#define CATCH_CONFIG_MAIN

#include <thirdpart/catch.hpp>

 

TEST_CASE("insert elements", "[iterator][insert][emplace][emplace_hint]")
{

    SetT h;
    
    SECTION("insert")
    {
        REQUIRE(h.insert(1).second == true);
        REQUIRE(*(h.insert(1).first) == 1);
        REQUIRE(h.insert(1).second == false);
    }

    SECTION("emplace")
    {
        REQUIRE(h.emplace(1).second == true);
        REQUIRE(*(h.emplace(1).first) == 1);
        REQUIRE(h.emplace(1).second == false);
    }

    SECTION("insert with iterator")
    {
        auto values = { 1, 2, 3, 4, 5 };
        std::ranges::copy(values, std::inserter(h, h.end()));
        REQUIRE(std::ranges::distance(h) == 5);
    }

    SECTION("emplace with iterator")
    {
        auto values = { 1, 2, 3, 4, 5 };
        std::ranges::for_each(values, [&](int x) {
            h.emplace_hint(h.end(), x);
        });
        REQUIRE(std::ranges::distance(h) == 5);
    }

}

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

TEST_CASE("member type", "[concept or type]")
{
    // https://en.cppreference.com/w/cpp/container/set
    using key_type = int;
    using value_type = int;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = std::less<>;
    using value_compare = std::less<>;
    using allocator_type = std::allocator<int>;
    using reference = int&;
    using const_reference = const int&;
    using pointer = std::allocator_traits<allocator_type>::pointer;
    using const_pointer = std::allocator_traits<allocator_type>::const_pointer;
    using SetType = SetT;

#define CheckTypeIsEqual(type) REQUIRE(std::is_same_v< typename SetType:: type, type >)

    CheckTypeIsEqual(key_type);
    CheckTypeIsEqual(value_type);
    CheckTypeIsEqual(size_type);
    CheckTypeIsEqual(difference_type);
    CheckTypeIsEqual(key_compare);
    CheckTypeIsEqual(value_compare);
    CheckTypeIsEqual(allocator_type);
    CheckTypeIsEqual(reference);
    CheckTypeIsEqual(const_reference);
    CheckTypeIsEqual(pointer);
    CheckTypeIsEqual(const_pointer);

#undef CheckTypeIsEqual

    REQUIRE(std::ranges::bidirectional_range<SetType>);

    REQUIRE(std::is_nothrow_move_constructible_v<SetT>);
    REQUIRE(std::is_nothrow_move_assignable_v<SetT>);
}

TEST_CASE("random test", "[contains][insert][erase]")
{
    std::random_device rd;

    auto random_range_int = [&](int n = 100000)
    {
        auto random_generator = [&]() {
            return rd() % (10 * n);
        };
        std::vector<int> ret;
        ret.reserve(n);
        std::generate_n(std::back_inserter(ret), n, random_generator);
        return ret;
    };

    auto dataset1 = random_range_int();
    auto dataset2 = random_range_int();
    auto dataset3 = random_range_int();

    SetT s1;
    std::set<int> s2;

    for (auto value : dataset1)
        s1.insert(value), s2.insert(value);

    REQUIRE(std::ranges::equal(s1, s2));

    int count1 = 0, count2 = 0;

    for (auto value : dataset2)
        count1 += s1.contains(value), count2 += s2.contains(value);

    REQUIRE(count1 == count2);

    for (auto value : dataset3)
        count1 += s1.erase(value), count2 += s2.erase(value);

    REQUIRE(std::ranges::equal(s1, s2));

}


TEST_CASE("operator[]", "[operator]")
{
    MapT m;

    m[1] = "Hello";
    m[2] = "World";
    m[3] = "!";

    REQUIRE(m.find(1) != m.end());
    REQUIRE(m.size() == 3);
    REQUIRE(m.contains(3));

    m[3] = "!!";

    REQUIRE(m.find(3)->second == "!!");
}

TEST_CASE("map member type", "[concept or type]")
{
    // https://en.cppreference.com/w/cpp/container/map
    using key_type = int;
    using value_type = std::pair<const int, std::string>;
    using mapped_type = std::string;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type = std::allocator< std::pair<const int, std::string> >;
    using reference = std::pair<const int, std::string>&;
    using const_reference = const std::pair<const int, std::string>&;
    using pointer = std::allocator_traits<allocator_type>::pointer;
    using const_pointer = std::allocator_traits<allocator_type>::const_pointer;
    using key_compare = std::less<>;
    using MapType = MapT;

#define CheckTypeIsEqual(type) REQUIRE(std::is_same_v< typename MapType:: type, type >)

    CheckTypeIsEqual(key_type);
    CheckTypeIsEqual(value_type);
    CheckTypeIsEqual(mapped_type);
    CheckTypeIsEqual(size_type);
    CheckTypeIsEqual(difference_type);
    CheckTypeIsEqual(allocator_type);
    CheckTypeIsEqual(reference);
    CheckTypeIsEqual(const_reference);
    CheckTypeIsEqual(pointer);
    CheckTypeIsEqual(const_pointer);
    CheckTypeIsEqual(key_compare);

#undef CheckTypeIsEqual

    REQUIRE(std::ranges::bidirectional_range<MapType>);
}

TEST_CASE("swap", "[swap]")
{
    SetT h1, h2;
    h1.insert(0);
    h1.insert(1);
    h1.insert(2);
    h2.insert(-1);

    h1.swap(h2);

    REQUIRE(h1.size() == 1);
    REQUIRE(h1.contains(-1));

    REQUIRE(h2.size() == 3);
    REQUIRE(h2.contains(0));
    REQUIRE(h2.contains(1));
    REQUIRE(h2.contains(2));

    REQUIRE(noexcept(h1.swap(h2)));

}


// #include "struct.hpp"
// #include "except_allocator.hpp"
// #include "fancy_ptr.hpp"

#include <lv_cpp/struct.hpp>
#include <lv_cpp/fancy_ptr.hpp>
#include <lv_cpp/record_allocator.hpp>

TEST_CASE("element destroy", "[dtor]")
{
    using U = ::leviathan::collections::tree_set<Int32<>, std::less<>, std::allocator<Int32<>>, TreeNodeT>;
    {
        U h1, h2, h3;

        for (int i = 0; i < 10; ++i)
        {
            h1.insert(Int32<>(i));
            h1.emplace(i);
        }
        h2.emplace(0);
        h3.emplace(0);
        h1 = h2; // copy assign
        U moved{ std::move(h1) }; // move ctor
        h3 = std::move(moved); // move assign
    }

    auto a = Int32<>::total_construct();
    auto b = Int32<>::total_destruct();

    REQUIRE(a == b);
}

TEST_CASE("fancy pointer")
{
    ::leviathan::collections::tree_set<int, std::less<>, TrivialAllocator<int>, TreeNodeT> t;
    t.insert(0);
    t.find(0);
    t.erase(0);
}

TEST_CASE("memory", "[dtor]")
{
    {
        ::leviathan::collections::tree_set<int, std::less<>, RecordAllocator<int>, TreeNodeT> h1, h2;

        for (int i = 0; i < 10; ++i)
        {
            h1.insert(i);
            h1.emplace((long long)i);
        }
    }
    REQUIRE(CheckMemoryAlloc());
}

