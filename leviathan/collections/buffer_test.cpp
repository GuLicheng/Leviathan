#include <iostream>
#include <string>
#include <vector>
#include <format>
#include <list>
#include <random>
#include <algorithm>

#include "buffer.hpp"
#include <catch2/catch_all.hpp>

// #include <thirdpart/catch_amalgamated.hpp>

using AllocatorT = std::allocator<int>;
AllocatorT allocator;

TEST_CASE("test_default_constructor")
{
    cpp::collections::buffer<int> buffer;
    
    REQUIRE(buffer.empty());
    REQUIRE(buffer.size() == 0);
    REQUIRE(buffer.capacity() == 0);

    buffer.emplace_back(allocator, 0);

    REQUIRE(*buffer.begin() == 0);
    REQUIRE(!buffer.empty());
    REQUIRE(buffer.size() == 1);
    REQUIRE(buffer.capacity() != 0);

    buffer.dispose(allocator);
}

TEST_CASE("test_constructor_with_capacity")
{
    cpp::collections::buffer<int> buffer(allocator, 24);

    REQUIRE(buffer.size() == 0);
    REQUIRE(buffer.capacity() == 32);

    buffer.dispose(allocator);
}

TEST_CASE("buffer_reverse")
{
    cpp::collections::buffer<int> buffer;

    REQUIRE(buffer.size() == 0);
    REQUIRE(buffer.capacity() == 0);

    buffer.reserve(allocator, 4);
    REQUIRE(buffer.size() == 0);
    REQUIRE(buffer.capacity() == 4);

    buffer.reserve(allocator, 24);

    REQUIRE(buffer.size() == 0);
    REQUIRE(buffer.capacity() == 32);

    buffer.reserve(allocator, 12);
    REQUIRE(buffer.size() == 0);
    REQUIRE(buffer.capacity() == 32);

    buffer.clear(allocator);

    buffer.dispose(allocator);
}

TEST_CASE("emplace_back_and_pop_back")
{
    cpp::collections::buffer<int> buffer;

    for (int i = 0; i < 10; ++i)
    {
        buffer.emplace_back(allocator, i);
    }

    REQUIRE(buffer.size() == 10);
    
    for (int i = 0; i < 10; ++i)
    {
        REQUIRE(buffer.begin()[i] == i);
    }

    buffer.pop_back(allocator);
    buffer.pop_back(allocator);
    buffer.pop_back(allocator);
    buffer.pop_back(allocator);
    buffer.pop_back(allocator);

    REQUIRE(buffer.size() == 5);

    for (int i = 0; i < 5; ++i)
    {
        REQUIRE(buffer.begin()[i] == i);
    }
    buffer.dispose(allocator);
}

TEST_CASE("iterator")
{
    using TBuffer = cpp::collections::buffer<int>;

    REQUIRE(std::ranges::contiguous_range<TBuffer>);

    cpp::collections::buffer<int> buffer;

    for (int i = 0; i < 10; ++i)
    {
        buffer.emplace_back(allocator, i);
    }
    
    int i = 0;

    for (auto value : buffer)
    {
        REQUIRE(value == i++);
    }

    buffer.dispose(allocator);
}

TEST_CASE("insert")
{
    using StringAlloc = std::allocator<std::string>;

    StringAlloc salloc;

    cpp::collections::buffer<std::string> buffer;

    buffer.emplace(salloc, buffer.end(), "David");

    buffer.emplace(salloc, buffer.begin(), "Cindy");
    buffer.emplace(salloc, buffer.begin(), "Bob");
    buffer.emplace(salloc, buffer.begin(), "Alice");

    REQUIRE(buffer.size() == 4);
    REQUIRE(buffer.capacity() == 4);
    
    REQUIRE(buffer[0] == "Alice");
    REQUIRE(buffer[1] == "Bob");
    REQUIRE(buffer[2] == "Cindy");
    REQUIRE(buffer[3] == "David");

    REQUIRE(buffer.front() == "Alice");
    REQUIRE(buffer.back() == "David");

    buffer.dispose(salloc);
}

TEST_CASE("erase_element")
{
    cpp::collections::buffer<int> buffer;

    for (int i = 0; i < 10; ++i)
    {
        buffer.emplace_back(allocator, i); 
    } // [0, 1, ..., 9]

    auto ret = buffer.erase(allocator, buffer.cbegin());

    REQUIRE(*ret == 1);

    ret = buffer.erase(allocator, buffer.cend() - 1);

    REQUIRE(ret == buffer.end());

    REQUIRE(buffer.size() == 8);

    REQUIRE(std::ranges::contiguous_range<cpp::collections::buffer<int>>);

    auto ilist = { 1, 2, 3, 4, 5, 6, 7, 8 };

    REQUIRE(std::ranges::equal(buffer, ilist));

    buffer.dispose(allocator);
}

TEST_CASE("erase_range")
{
    cpp::collections::buffer<int> buffer;

    for (int i = 0; i < 10; ++i)
    {
        buffer.emplace_back(allocator, i); 
    } // [0, 1, ..., 9]

    auto ret = buffer.erase(allocator, buffer.end(), buffer.end());

    REQUIRE(ret == buffer.cend());
    REQUIRE(buffer.size() == 10);
    // Remove last two elements

    ret = buffer.erase(allocator, buffer.end() - 2, buffer.end());
    auto ilist1 = { 0, 1, 2, 3, 4, 5, 6, 7 };

    REQUIRE(std::ranges::equal(buffer, ilist1));
    REQUIRE(ret == buffer.end());

    // Remove first two elements
    ret = buffer.erase(allocator, buffer.begin(), buffer.begin() + 2);
    auto ilist2 = { 2, 3, 4, 5, 6, 7 };
    REQUIRE(std::ranges::equal(buffer, ilist2));
    REQUIRE(*ret == 2);

    buffer.dispose(allocator);
}

TEST_CASE("insert_range")
{
    cpp::collections::buffer<int> buffer;

    // Insert empty range
    std::initializer_list<int> empty_list{};
    auto ret = buffer.insert(allocator, buffer.end(), empty_list.begin(), empty_list.end());
    
    REQUIRE(ret == buffer.end());
    REQUIRE(buffer.size() == 0);

    // Insert element at end
    auto ilist = { 1, 2, 3 };
    ret = buffer.insert(allocator, buffer.end(), ilist.begin(), ilist.end());

    REQUIRE(std::ranges::equal(buffer, ilist));
    REQUIRE(buffer.size() == ilist.size());
    REQUIRE(*ret == 1);

    // Insert element at begin
    ret = buffer.insert(allocator, buffer.begin(), ilist.begin(), ilist.end());
    auto ilist2 = { 1, 2, 3, 1, 2, 3 };
    
    REQUIRE(std::ranges::equal(buffer, ilist2));
    REQUIRE(buffer.size() == ilist.size() * 2);
    REQUIRE(*ret == 1);

    // Insert element at middle
    ret = buffer.insert(allocator, buffer.begin() + 1, ilist.begin(), ilist.end());

    auto ilist3 = { 1, 1, 2, 3, 2, 3, 1, 2, 3 };
    REQUIRE(std::ranges::equal(buffer, ilist3));  
    REQUIRE(buffer.size() == ilist.size() * 3);
    REQUIRE(*ret == 1);

    // Insert bidirectional range
    std::list<int> bidirectional_list = { -1, -2, -3 };

    ret = buffer.insert(allocator, buffer.cbegin(), bidirectional_list.begin(), bidirectional_list.end());

    REQUIRE(buffer.size() == ilist.size() * 3 + 3);
    REQUIRE(*ret == -1);
    REQUIRE(buffer[0] == -1);
    REQUIRE(buffer[1] == -2);
    REQUIRE(buffer[2] == -3);

    buffer.dispose(allocator);
}

TEST_CASE("random_insert")
{
    std::random_device rd;
    std::vector<int> numbers;
    cpp::collections::buffer<int> buffer;
    
    for (int i = 0; i < 100'000; ++i)
    {
        auto idx = numbers.empty() ? 0 : rd() % numbers.size();
        numbers.emplace(numbers.begin() + idx, i);
        buffer.emplace(allocator, buffer.begin() + idx, i);
    }

    REQUIRE(numbers.size() == buffer.size());

    REQUIRE(std::equal(
        numbers.begin(), numbers.end(), 
        buffer.begin()
    ));

    buffer.dispose(allocator);
}

TEST_CASE("insert_element_from_self")
{
    using StringAllocator = std::allocator<std::string>;
    cpp::collections::buffer<std::string> buffer;

    StringAllocator salloc;

    buffer.emplace(salloc, buffer.begin(), "This sentence must be longer enough and will be moved to last position");

    buffer.emplace(salloc, buffer.begin(), buffer[0]);
    buffer.emplace(salloc, buffer.begin(), buffer[0]);

    REQUIRE(buffer.size() == 3);
    REQUIRE(buffer[0] == "This sentence must be longer enough and will be moved to last position");
    REQUIRE(buffer[1] == "This sentence must be longer enough and will be moved to last position");
    REQUIRE(buffer[2] == "This sentence must be longer enough and will be moved to last position");

    buffer.dispose(salloc);
}

TEST_CASE("emplace_back_element_from_self")
{
    using StringAllocator = std::allocator<std::string>;
    cpp::collections::buffer<std::string> buffer;

    StringAllocator salloc;

    buffer.emplace_back(salloc, "This sentence must be longer enough.");

    buffer.emplace_back(salloc, buffer.front());
    buffer.emplace_back(salloc, buffer.front());
    buffer.emplace_back(salloc, buffer.front());

    REQUIRE(buffer.size() == 4);
    REQUIRE(buffer[0] == "This sentence must be longer enough.");
    REQUIRE(buffer[1] == "This sentence must be longer enough.");
    REQUIRE(buffer[2] == "This sentence must be longer enough.");
    REQUIRE(buffer[3] == "This sentence must be longer enough.");

    buffer.dispose(salloc);
}

TEST_CASE("initializer_list")
{
    cpp::collections::buffer<int> buffer(allocator, { 1, 2, 3 });
    REQUIRE(buffer.size() == 3);
    REQUIRE(buffer.capacity() == 4);
    REQUIRE(buffer[0] == 1);
    REQUIRE(buffer[1] == 2);
    REQUIRE(buffer[2] == 3);
    buffer.dispose(allocator);
}

// #include "leviathan/struct.hpp"
#include <leviathan/utils/controllable_value.hpp>

TEST_CASE("exception")
{
    using T = cpp::controllable_value<int, 2, -1>;
    // using T = Int32<false, 2, -1, true>;
    {
        std::allocator<T> alloc;

        cpp::collections::buffer<T> buffer(alloc, 1);

        REQUIRE(std::is_nothrow_move_constructible_v<T> == false);
        REQUIRE(!T::moveable);

        bool is_throw = false;

        try 
        {
            for (int i = 0; i < 10; ++i)
            {
                buffer.emplace_back(alloc, i);
            }
        }
        catch(...)
        {
            is_throw = true;
        }
        REQUIRE(is_throw == true);
        buffer.dispose(alloc);
    }

    REQUIRE(T::total_construct() == T::total_destruct());
}

#include <memory_resource>

TEST_CASE("pmr::allocator")
{
    using PmrAllocator = std::pmr::polymorphic_allocator<int>;
    using Buffer = cpp::collections::buffer<int>;

    Buffer buffer;

    PmrAllocator pa;

    buffer.emplace_back(pa, 0);
    buffer.emplace_back(pa, 1);

    REQUIRE(buffer[0] == 0);
    REQUIRE(buffer[1] == 1);

    buffer.dispose(pa);
}






