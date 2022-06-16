#pragma once

#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <lv_cpp/utils/timer.hpp>
#include <thirdpart/catch.hpp>
#include <assert.h>
#include <random>
#include <algorithm> 
#include <vector> // store elements
#include <ranges>

#ifndef DEFAULT_NUM
#define DEFAULT_NUM 1'00'000
#endif

#define SplitLine() (std::cout << "===============================================\n")

namespace leviathan::test
{

    // We use algorithm + std::ranges to replace Catch2's Generator 

    namespace detail
    {
        inline std::random_device rd;

        inline constexpr auto default_num = DEFAULT_NUM; // DEFAULT_NUM x Catch::DataConfig::benchmarkSamples

        std::vector<int> random_range_int(int n = default_num)
        {
            auto random_generator = [&]() {
                return rd();
            };
            std::vector<int> ret;
            ret.reserve(n);
            std::generate_n(std::back_inserter(ret), n, random_generator);
            return ret;
        }

        std::vector<int> random_ascending(int n = default_num)
        {
            std::vector<int> vec;
            vec.reserve(n);
            std::ranges::copy(std::views::iota(0, n), std::back_inserter(vec));
            return vec;
        }

        std::vector<int> random_descending(int n = default_num)
        {
            std::vector<int> vec;
            vec.reserve(n);
            std::ranges::copy(std::views::iota(0, n) | std::views::reverse, std::back_inserter(vec));
            return vec;
        }

        std::vector<int> fill(int value, int n = default_num)
        {
            return std::vector<int>(n, value);
        }

        std::vector<int> pipe_organ(int n = default_num) 
        {
            std::vector<int> v; v.reserve(n);
            for (int i = 0; i < n / 2; ++i) v.push_back(i);
            for (int i = n / 2; i < n; ++i) v.push_back(n - i);
            return v;
        }
    }    

    inline namespace insertion
    {
        inline auto ascending = detail::random_ascending(); 
        inline auto descending = detail::random_descending(); 
        inline auto random_int = detail::random_range_int(); 
    }

    inline namespace search
    {
        inline auto searching = detail::random_range_int();
    }

    inline namespace removing
    {
        inline auto remove = detail::random_range_int();
    }

    template <typename OrderedSet>
    auto random_insert_test()
    {
        OrderedSet s;
        for (auto val : insertion::random_int) s.insert(val);
        assert(s.size() <= detail::default_num);
        return s.size();
    }

    template <typename OrderedSet>
    auto ascending_insert_test()
    {
        OrderedSet s;
        for (auto val : insertion::ascending) s.insert(val);
        assert(s.size() <= detail::default_num);
        return s.size();
    }

    template <typename OrderedSet>
    auto descending_insert_test()
    {
        OrderedSet s;
        for (auto val : insertion::descending) s.insert(val);
        assert(s.size() <= detail::default_num);
        return s.size();
    }

    template <typename OrderedSet>
    auto search_test(const OrderedSet& s)
    {

        REQUIRE(s.size() > 0);

        int cnt = 0;

        for (auto val : search::searching) cnt += s.contains(val);

        REQUIRE(cnt <= detail::default_num);

        return cnt;
    }

    template <typename OrderedSet>
    auto remove_test(OrderedSet& s)
    {
        REQUIRE(s.size() > 0);

        for (auto val : removing::remove) s.erase(val);

        return s.size();
    }

    template <typename... OrderedSets>
    void random_insert(OrderedSets&... s)
    {
        bool is_all_empty = (s.empty() && ...); 
        REQUIRE(is_all_empty);
        for (auto val : insertion::random_int) 
        {
            (s.insert(val), ...);   
        }
    }

    template <typename... OrderedMaps>
    void random_insert_map(OrderedMaps&... s)
    {
        bool is_all_empty = (s.empty() && ...); 
        REQUIRE(is_all_empty);
        for (auto val : insertion::random_int) 
        {
            (s.insert({val, std::to_string(val)}), ...);  
        }
    }

    template <typename Map>
    auto random_insert_test2()
    {
        Map s;
        for (auto val : insertion::random_int) s.insert({val, val});
        assert(s.size() <= detail::default_num);
        return s.size();
    }

    template <typename Map>
    auto ascending_insert_test2()
    {
        Map s;
        for (auto val : insertion::ascending) s.insert({val, val});
        assert(s.size() <= detail::default_num);
        return s.size();
    }

    template <typename Map>
    auto descending_insert_test2()
    {
        Map s;
        for (auto val : insertion::descending) s.insert({val, val});
        assert(s.size() <= detail::default_num);
        return s.size();
    }


}





