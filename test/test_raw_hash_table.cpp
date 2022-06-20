#define CATCH_CONFIG_MAIN

#include <thirdpart/catch.hpp>

#include <lv_cpp/collections/internal/raw_hash_table.hpp>
#include <algorithm>

using HashT = ::leviathan::collections::hash_set<int>;

TEST_CASE("hash_iterator", "[iterator]")
{
    HashT h;
    auto first = h.begin();
    auto last = h.end();
    REQUIRE(std::ranges::distance(first, last) == 0);
}

TEST_CASE("insert elements", "[iterator][insert][emplace]")
{
    HashT h;
    
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

TEST_CASE("observer", "[empty][capacity][size]")
{
    HashT h;
    REQUIRE(h.capacity() == 0);
    REQUIRE(h.size() == 0);
    REQUIRE(h.empty());
}

TEST_CASE("search elements", "[contains][find][iterator]")
{
    HashT h;

    REQUIRE(h.find(0) == h.end());
    REQUIRE(!h.contains(0));

    h.insert(1);
    h.insert(3);

    REQUIRE(h.find(1) != h.end());
    REQUIRE(h.find(2) == h.end());
    REQUIRE(h.find(3) != h.end());

    REQUIRE(h.contains(1));
    REQUIRE(!h.contains(2));
    REQUIRE(h.contains(3));

}

TEST_CASE("remove elements", "[iterator][remove][clear]")
{
    HashT h;

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

    auto iter = h.begin();
    while (iter != h.end()) iter = h.erase(iter);

    REQUIRE(h.size() == 0);

}

TEST_CASE("member type", "[concept or type]")
{
    // https://en.cppreference.com/w/cpp/container/set
    using key_type = int;
    using value_type = int;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type = std::allocator<int>;
    using reference = int&;
    using const_reference = const int&;
    using pointer = std::allocator_traits<allocator_type>::pointer;
    using const_pointer = std::allocator_traits<allocator_type>::const_pointer;
    using hasher = std::hash<::leviathan::collections::auto_hash>;
    using key_equal = std::equal_to<void>;
    using SetType = ::leviathan::collections::hash_set<int>;

#define CheckTypeIsEqual(type) REQUIRE(std::is_same_v< typename SetType:: type, type >)

    CheckTypeIsEqual(key_type);
    CheckTypeIsEqual(value_type);
    CheckTypeIsEqual(size_type);
    CheckTypeIsEqual(difference_type);
    CheckTypeIsEqual(allocator_type);
    CheckTypeIsEqual(reference);
    CheckTypeIsEqual(const_reference);
    CheckTypeIsEqual(pointer);
    CheckTypeIsEqual(const_pointer);
    CheckTypeIsEqual(hasher);
    CheckTypeIsEqual(key_equal);

#undef CheckTypeIsEqual

    REQUIRE(std::ranges::forward_range<SetType>);

}

TEST_CASE("ctor, assign and swap", "[ctor][swap][assign]")
{
    HashT h1;

    REQUIRE(std::is_nothrow_move_constructible_v<HashT>);
    REQUIRE(std::is_nothrow_move_assignable_v<HashT>);

    h1.insert(1);
    h1.insert(2);
    h1.insert(3);


    SECTION("copy ctor")
    {
        HashT h2 = h1;
        REQUIRE(std::ranges::equal(h1, h2));
    }

    SECTION("move ctor")
    {
        HashT h2 = std::move(h1);
        REQUIRE(h2.contains(1));
        REQUIRE(h2.contains(2));
        REQUIRE(h2.contains(3));
        REQUIRE(h1.empty());
    }

    SECTION("swap")
    {
        HashT h2;
        h2.insert(-1);
        h2.insert(-2);

        h1.swap(h2);

        REQUIRE(h1.size() == 2);
        REQUIRE(h1.contains(-1));
        REQUIRE(h1.contains(-2));
        REQUIRE(!h1.contains(1));
        REQUIRE(!h1.contains(2));
        REQUIRE(!h1.contains(3));
    }

    SECTION("copy assign")
    {
        HashT h2;
        h2 = h1;
        REQUIRE(std::ranges::equal(h1, h2));
    }

    SECTION("move assign")
    {
        HashT h2 = std::move(h1);    
        REQUIRE(h1.empty());
        REQUIRE(h2.contains(1));
        REQUIRE(h2.contains(2));
        REQUIRE(h2.contains(3));
        REQUIRE(h2.size() == 3);
    }

}


#include <lv_cpp/utils/struct.hpp>

TEST_CASE("element destroy", "[dtor]")
{

    {
        ::leviathan::collections::hash_set<Int32<>> h;

        // rehash
        for (int i = 0; i < 10; ++i)
        {
            h.insert(Int32<>(i));
            h.emplace(i);
        }
    }

    auto a = Int32<>::total_construct();
    auto b = Int32<>::total_destruct();

    REQUIRE(a == b);
}




