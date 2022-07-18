#include <lv_cpp/collections/internal/avl_tree.hpp>
#include <memory_resource>
#include <iostream>
#include <random>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <compare>
#include <set>
#include <chrono>
#include <ranges>
#include <vector>
#include <string_view>
#include <lv_cpp/utils/timer.hpp>
#include <bits/stl_tree.h>
#include <map>
#include "tree1.hpp"

using leviathan::collections::avl_set;
using leviathan::collections::pmr_avl_set;
using T = avl_set<int>;

std::vector<int> random_number()
{
    std::vector<int> ret;
    static std::random_device rd;
    std::generate_n(std::back_inserter(ret), 1000'000, [](){
        return rd() % 1000'000;
    });
    return ret;
}

void test()
{
    system("chcp 65001");

    std::set<int> s1;
    T s2;
    int cnt1 = 0;
    int cnt2 = 0;
    auto numbers1 = random_number();
    auto numbers2 = random_number();
    auto numbers3 = random_number();
    {
        ::leviathan::timer _;
        for (auto x : numbers1)
            s1.insert(x);
        for (auto x : numbers3)
            cnt1 += s1.contains(x);
        for (auto x : numbers2)
            s1.erase(x);
    }
    {
        ::leviathan::timer _;
        for (auto x : numbers1)
            s2.insert(x);
        for (auto x : numbers3)
            cnt2 += s2.contains(x);
        for (auto x : numbers2)
            s2.erase(x);
    }

    assert(std::ranges::equal(s1, s2));
    assert(cnt1 == cnt2);
}

int main()
{
    test();
    std::cout << "Ok\n";
}





