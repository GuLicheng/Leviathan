#pragma once

#include <lv_cpp/utils/timer.hpp>
#include <assert.h>
#include <random>
#include <algorithm> 
#include <vector> // store elements
#include <ranges>

#define SplitLine() (std::cout << "===============================================\n")

namespace leviathan::test
{

    namespace detail
    {
        inline std::random_device rd;

        inline constexpr auto default_num = 1'000'000;

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
    void insert_test(std::string info)
    {
        SplitLine();
        std::cout << "InsertTesting\n";
        {
            OrderedSet s;
            leviathan::timer _{info + " ascending"};
            for (auto val : insertion::ascending) s.insert(val);
            assert(s.size() == detail::default_num);
        }

        {
            OrderedSet s;
            leviathan::timer _{info + " descending"};
            for (auto val : insertion::descending) s.insert(val);
            assert(s.size() == detail::default_num);
        }

        {
            OrderedSet s;
            leviathan::timer _{info + " random"};
            for (auto val : insertion::random_int) s.insert(val);
            assert(s.size() <= detail::default_num);
        }

    }

    template <typename OrderedSet>
    void search_test(std::string info)
    {
        SplitLine();

        std::cout << "SearchingTesting\n";
        
        OrderedSet s;

        for (auto val : insertion::random_int) s.insert(val);

        int cnt = 0;

        leviathan::timer _{info + " random search"};

        for (auto val : search::searching) cnt += s.contains(val);

        assert(cnt <= detail::default_num);

    }

    template <typename OrderedSet>
    void remove_test(std::string info)
    {
        SplitLine();

        std::cout << "RemovingTesting\n";

        {
            OrderedSet s;
            for (auto val : insertion::random_int) s.insert(val);
            leviathan::timer _{info + " ascending"};
            for (auto val : insertion::ascending) s.erase(val);
            assert(s.size() <= detail::default_num);
        }

        {
            OrderedSet s;
            for (auto val : insertion::random_int) s.insert(val);
            leviathan::timer _{info + " descending"};
            for (auto val : insertion::descending) s.erase(val);
            assert(s.size() <= detail::default_num);
        }

        {
            OrderedSet s;
            for (auto val : insertion::random_int) s.insert(val);
            leviathan::timer _{info + " random_int"};
            for (auto val : removing::remove) s.erase(val);
            assert(s.size() <= detail::default_num);
        }
    }


    template <typename OrderedSet>
    void all_test(std::string info)
    {
        insert_test<OrderedSet>(info);
        search_test<OrderedSet>(info);
        remove_test<OrderedSet>(std::move(info));
    }

}





