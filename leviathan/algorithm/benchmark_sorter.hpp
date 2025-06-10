#pragma once

#include <chrono>
#include <print>
#include <random>
#include <vector>
#include <fstream>
#include <string>
#include <string_view>
#include <concepts>
#include <algorithm>
#include <leviathan/meta/type.hpp>

namespace cpp
{

template <std::default_initializable... Sorters>
struct benchmark_sorter
{
    using compare = std::ranges::less;

    static constexpr std::string_view line = "----------------------------------------------------------------------";
    static constexpr auto line_length = line.size();

    std::vector<std::string_view> m_names;
    size_t m_max_name_length = 0;

    benchmark_sorter() = default;

    benchmark_sorter(std::vector<std::string_view> names)
    {
        assert(!names.empty() && "Names vector must not be empty.");
        m_names = std::move(names);
        auto Size = [](auto x) static { return x.size(); };
        m_max_name_length = std::ranges::max(m_names | std::views::transform(Size) | std::views::cache_latest);
    }

    template <typename Sorter>
    auto add_sorter(Sorter sorter, std::string_view name)
    {
        m_names.emplace_back(name);
        return benchmark_sorter<Sorters..., Sorter>(std::move(m_names));
    }

    using clock = std::chrono::high_resolution_clock;

    template <typename Range>
    benchmark_sorter& operator()(Range numbers, std::string_view distribution = "random") 
    {
        std::print("{}\n{}\n{}\n", line, distribution, line);
        benchmarks(numbers, std::make_index_sequence<sizeof...(Sorters)>{});
        std::print("\n\n\n");
        return *this;
    }   

    benchmark_sorter& ascending(int num = 1e7, std::string_view distribution = "ascending") 
    {
        std::vector<int> asce(std::from_range, std::views::iota(0, 1000000));
        return (*this)(std::move(asce), distribution);
    }

    benchmark_sorter& descending(int num = 1e7, std::string_view distribution = "descending") 
    {
        std::vector<int> desc(std::from_range, std::views::iota(0, 1000000) | std::views::reverse);
        return (*this)(std::move(desc), distribution);
    }

    benchmark_sorter& all_zeros(int num = 1e7, std::string_view distribution = "all zeros") 
    {
        std::vector<int> all_zeros(num, 0);
        return (*this)(std::move(all_zeros), distribution);
    }

    benchmark_sorter& random(int num = 1e7, std::string_view distribution = "random") 
    {
        std::vector<int> random_numbers(num);
        // std::generate(random_numbers.begin(), random_numbers.end(), std::mt19937{std::random_device{}()});
        std::generate(random_numbers.begin(), random_numbers.end(), std::mt19937{0});
        return (*this)(std::move(random_numbers), distribution);
    }

    benchmark_sorter& pipeline(int num = 1e7, std::string_view distribution = "pipeline") 
    {
        std::vector<int> pipeline_numbers(num);
        std::iota(pipeline_numbers.begin(), pipeline_numbers.end(), 0);
        std::ranges::reverse(pipeline_numbers | std::views::drop(num / 2));
        return (*this)(std::move(pipeline_numbers), distribution);
    }

    template <typename Range, size_t... Idx>
    void benchmarks(const Range& numbers, std::index_sequence<Idx...>) 
    {
        (benchmark<Idx>(numbers), ...);
    }

    template <size_t I, typename Range>
    void benchmark(Range numbers) 
    {
        const auto tp1 = clock::now();
        Sorters...[I]()(numbers, compare());
        const auto tp2 = clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(tp2 - tp1).count();
        std::println("{0:>{3}} took {1:>5} milliseconds. correct ? {2}.\n{4}", 
            m_names[I], duration, std::ranges::is_sorted(numbers, compare()), m_max_name_length, line);
    }
};


} // namespace cpp

