#include <ranges>
#include <catch2/catch_all.hpp>
#include <iostream>

constexpr int N = 1024 * 4;

inline std::vector<int> v1(N, 1);
inline std::vector<int> v2(N, 1);

TEST_CASE("benchmark for-loop and views")
{
    BENCHMARK_ADVANCED("for-loop")(Catch::Benchmark::Chronometer meter) {
        meter.measure([]{
            int s = 0;
            for (auto&& i : v1)
            {
                for (auto&& j : v2)
                {
                    s += i + j;
                }
            }
            return s;
        });
    };

    BENCHMARK_ADVANCED("cartesian_product")(Catch::Benchmark::Chronometer meter) {
        meter.measure([]{
            int s = 0;
            for (auto&& ij : std::views::cartesian_product(v1, v2))
            {
                s += ij.first + ij.second;
            }
            return s;
        });
    };
}

