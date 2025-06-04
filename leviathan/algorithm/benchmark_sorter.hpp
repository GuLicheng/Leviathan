#pragma once

#include <chrono>
#include <print>
#include <random>
#include <vector>
#include <fstream>
#include <string>
#include <concepts>
#include <leviathan/meta/type.hpp>

namespace cpp
{

template <std::default_initializable... Sorters>
struct benchmark_sorter
{
    using compare = std::ranges::less;

    std::vector<const char*> m_names;

    benchmark_sorter() = default;

    benchmark_sorter(std::vector<const char*> names)
    {
        m_names = std::move(names);
    }

    template <typename Sorter>
    auto add_sorter(Sorter sorter, const char* name)
    {
        m_names.emplace_back(name);
        return benchmark_sorter<Sorters..., Sorter>(std::move(m_names));
    }

    using clock = std::chrono::high_resolution_clock;

    template <typename Range>
    void operator()(const Range& numbers) 
    {
        benchmarks(numbers, std::make_index_sequence<sizeof...(Sorters)>{});
    }   

    template <typename Range, size_t... Idx>
    void benchmarks(const Range& numbers, std::index_sequence<Idx...>) 
    {
        (benchmark<Idx>(numbers), ...);
    }

    template <size_t I, typename Range>
    void benchmark(Range numbers) 
    {
        auto tp1 = clock::now();
        Sorters...[I]()(numbers, compare());
        auto tp2 = clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(tp2 - tp1).count();
        std::println("|{0:>20}| took {1} milliseconds. Is sorted ? {2}.", 
            m_names[I], duration, std::ranges::is_sorted(numbers, compare()));
    }
};


} // namespace cpp

