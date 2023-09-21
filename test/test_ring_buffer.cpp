#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <iostream>
#include <string>
#include <vector>
#include <format>
#include <list>
#include <random>
#include <algorithm>

#include <leviathan/collections/internal/ring_buffer.hpp>
// #include <catch2/catch_all.hpp>
#include <catch2/catch_all.hpp>

TEST_CASE("emplace front and back")
{
    leviathan::collections::ring_buffer<int> buffer;

    buffer.emplace_front(-1);
    buffer.emplace_front(-2);

    buffer.emplace_back(1);
    buffer.emplace_back(2);

    REQUIRE(buffer.size() == 4);
    REQUIRE(buffer.capacity() >= 4);

    REQUIRE(buffer.back() == 2);
    REQUIRE(buffer.front() == -2);

    REQUIRE(buffer[0] == -2);
    REQUIRE(buffer[1] == -1);
    REQUIRE(buffer[2] == 1);
    REQUIRE(buffer[3] == 2);

    leviathan::collections::ring_buffer<std::string> pool1, pool2;

    std::string strs[] = {
        "This sentence must be long enough so that it will be put on heap.",
    };

    pool1.emplace_back(strs[0]);
    pool1.emplace_back(pool1[0]);
    pool1.emplace_back(pool1[0]);

    pool2.emplace_front(strs[0]);
    pool2.emplace_front(pool2[0]);
    pool2.emplace_front(pool2[0]);

    REQUIRE(pool1[0] == strs[0]);
    REQUIRE(pool1[1] == strs[0]);
    REQUIRE(pool1[2] == strs[0]);
    
    REQUIRE(pool2[0] == strs[0]);
    REQUIRE(pool2[1] == strs[0]);
    REQUIRE(pool2[2] == strs[0]);
}

TEST_CASE("iterator")
{
    leviathan::collections::ring_buffer<int> rb;

    rb.emplace_back(1);
    rb.emplace_back(0);
    rb.emplace_back(2);
    rb.emplace_back(4);
    rb.emplace_back(6);
    rb.emplace_back(5);
    rb.emplace_back(3);

    std::ranges::sort(rb);

    for (size_t i = 0; i < rb.size(); ++i)
    {
        REQUIRE(rb[i] == i);
    }

    std::ranges::reverse(rb);

    for (size_t i = 0; i < rb.size(); ++i)
    {
        auto x = 6 - i;
        REQUIRE(rb[i] == x);
    }

    rb.clear();

    rb.emplace_back(0);
    rb.emplace_back(1);
    rb.emplace_back(2);
    rb.emplace_back(3);

    rb.pop_front();
    rb.pop_front();

    rb.emplace_back(-1);
    rb.emplace_back(-2);

    std::ranges::sort(rb);

    REQUIRE(rb[0] == -2);
    REQUIRE(rb[1] == -1);
    REQUIRE(rb[2] == 2);
    REQUIRE(rb[3] == 3);

    std::ranges::reverse(rb);

    REQUIRE(rb[0] == 3);
    REQUIRE(rb[1] == 2);
    REQUIRE(rb[2] == -1);
    REQUIRE(rb[3] == -2);

}

#include "leviathan/struct.hpp"

TEST_CASE("exception")
{
    using T = Int32<false, 2, -1, true>;
    {
        std::allocator<T> alloc;

        leviathan::collections::ring_buffer<T> buffer;

        REQUIRE(std::is_nothrow_move_constructible_v<T> == false);
        REQUIRE(!T::Moveable);

        bool is_throw = false;

        try 
        {
            for (int i = 0; i < 10; ++i)
            {
                buffer.emplace_back(i);
            }
        }
        catch(...)
        {
            is_throw = true;
        }
        REQUIRE(is_throw == true);
    }

    REQUIRE(T::total_construct() == T::total_destruct());
}



