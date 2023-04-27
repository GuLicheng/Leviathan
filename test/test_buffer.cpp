#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>

#include <lv_cpp/collections/internal/buffer.hpp>
#include <catch2/catch_all.hpp>

// #include <thirdpart/catch_amalgamated.hpp>

using AllocatorT = std::allocator<int>;
AllocatorT allocator;

TEST_CASE("test_default_constructor")
{
    leviathan::collections::buffer<int, AllocatorT> buffer;
    
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
    leviathan::collections::buffer<int, AllocatorT> buffer(allocator, 24);

    REQUIRE(buffer.size() == 0);
    REQUIRE(buffer.capacity() == 24);

    buffer.dispose(allocator);
}

TEST_CASE("buffer_reverse")
{
    leviathan::collections::buffer<int, AllocatorT> buffer;

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
}

TEST_CASE("emplace_back_and_pop_back")
{
    leviathan::collections::buffer<int, AllocatorT> buffer;

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
}

TEST_CASE("iterator")
{
    leviathan::collections::buffer<int, AllocatorT> buffer;

    for (int i = 0; i < 10; ++i)
    {
        buffer.emplace_back(allocator, i);
    }
    
    int i = 0;

    for (auto value : buffer)
    {
        REQUIRE(value == i++);
    }
}

TEST_CASE("insert")
{
    using StringAlloc = std::allocator<std::string>;

    StringAlloc salloc;

    leviathan::collections::buffer<std::string, StringAlloc> buffer;

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

    buffer.dispose(salloc);
}

TEST_CASE("random_insert")
{
    std::random_device rd;
    std::vector<int> numbers;
    leviathan::collections::buffer<int, AllocatorT> buffer;
    
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
}

#include "../include/lv_cpp/struct.hpp"

TEST_CASE("exception")
{
    using T = Int32<false, 2, -1, true>;
    {
        std::allocator<T> alloc;

        leviathan::collections::buffer<T, std::allocator<T>> buffer(alloc, 1);

        REQUIRE(std::is_nothrow_move_constructible_v<T> == false);
        REQUIRE(!T::Moveable);

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

#include <vector>

TEST_CASE("benchmark insert")
{
    constexpr int N = 1024;

    BENCHMARK_ADVANCED("std::vector")(Catch::Benchmark::Chronometer meter) {
        meter.measure([]{
            std::vector<int> vec;
            vec.reserve(N);
            for (int i = 0; i < N; ++i) 
                vec.emplace(vec.begin(), i);
            auto first = *vec.begin();
            return first;
        });
    };

    BENCHMARK_ADVANCED("buffer")(Catch::Benchmark::Chronometer meter) {
        meter.measure([]{
            leviathan::collections::buffer<int, AllocatorT> buffer;
            AllocatorT alloc;
            buffer.reserve(alloc, N);
            for (int i = 0; i < N; ++i) 
                buffer.emplace(alloc, buffer.begin(), i);
            auto first = *buffer.begin();
            buffer.dispose(alloc);
            return first;
        });
    };
}

TEST_CASE("benchmark emplace_back")
{
    constexpr int N = 1024 * 4;

    BENCHMARK_ADVANCED("std::vector")(Catch::Benchmark::Chronometer meter) {
        meter.measure([]{
            std::vector<int> vec;
            // vec.reserve(N);
            for (int i = 0; i < N; ++i) 
                vec.emplace_back(i);
            auto s = vec.size();
            return s;
        });
    };

    BENCHMARK_ADVANCED("buffer")(Catch::Benchmark::Chronometer meter) {
        meter.measure([]{
            leviathan::collections::buffer<int, AllocatorT> buffer;
            AllocatorT alloc;
            // buffer.reserve(alloc, N);
            for (int i = 0; i < N; ++i) 
                buffer.emplace_back(alloc, i);
            auto s = buffer.size();
            buffer.dispose(alloc);
            return s;
        });
    };
}
