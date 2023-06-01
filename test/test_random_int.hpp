#pragma once

#include <catch2/catch_all.hpp>

#include <set>
#include <vector>
#include <random>
#include <algorithm>
#include <type_traits>

namespace leviathan::test
{
    enum struct test_mode
    {
        InsertOnly,
        All
    };

    template <typename SetType, bool IsOrdered, test_mode Mode = test_mode::All>
    void test_set_is_correct()
    {

        static_assert(std::is_same_v<typename SetType::value_type, int>);

        constexpr int N = 100'0000;

        std::random_device rd;  

        std::vector<int> vec;

        for (int i = 0; i < N; ++i)
        {
            vec.emplace_back(rd() % (N * 10));
        }

        std::set<int> s1;
        SetType s2;

        for (auto value : vec)
        {
            auto op = rd() % 3;

            if constexpr (Mode == test_mode::InsertOnly)
                op = 0;

            switch (op)
            {
                case 0: 
                {
                    auto [_1, succeed1] = s1.insert(value);
                    auto [_2, succeed2] = s2.insert(value);
                    REQUIRE(succeed1 == succeed2);
                    break;
                }
                case 1:
                {
                    auto contains1 = s1.contains(value);
                    auto contains2 = s2.contains(value);
                    REQUIRE(contains1 == contains2);
                    break;
                }
                case 2:
                {
                    auto c1 = s1.erase(value);
                    auto c2 = s2.erase(value);
                    REQUIRE(c1 == c2);  
                    break;
                }
            }

        }

        if constexpr (IsOrdered)
        {
            REQUIRE(std::ranges::equal(s1, s2));
        }
        else
        {
            std::set<int> s3 { s2.begin(), s2.end() };
            REQUIRE(s1 == s3);
        }
    }
}
