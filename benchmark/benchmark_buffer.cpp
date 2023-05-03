#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <random>
#include <algorithm>

#include <lv_cpp/collections/internal/buffer.hpp>
#include <catch2/catch_all.hpp>
// #include <catch2/catch_test_macros.hpp>

// #include <thirdpart/catch_amalgamated.hpp>

using AllocatorT = std::allocator<int>;
AllocatorT allocator;

template <typename Fn>
struct RAII
{
    Fn m_fn;

    RAII(Fn fn) : m_fn(std::move(fn)) { }

    ~RAII()
    {
        m_fn();
    }
};

TEST_CASE("benchmark insert at first")
{
    constexpr int N = 1024 * 4;

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
            RAII guard([&](){ buffer.dispose(alloc); });
            buffer.reserve(alloc, N);
            for (int i = 0; i < N; ++i) 
                buffer.emplace(alloc, buffer.begin(), i);
            auto first = *buffer.begin();
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
            vec.reserve(N);
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
            buffer.reserve(alloc, N);
            RAII guard([&](){ buffer.dispose(alloc); });
            for (int i = 0; i < N; ++i) 
                buffer.emplace_back(alloc, i);
            auto s = buffer.size();
            return s;
        });
    };
}
