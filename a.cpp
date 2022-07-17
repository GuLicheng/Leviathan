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

using leviathan::collections::avl_set;
using leviathan::collections::pmr_avl_set;
using T = avl_set<int>;

std::vector<int> random_number()
{
    std::vector<int> ret;
    static std::random_device rd;
    std::generate_n(std::back_inserter(ret), 10'00'000, [](){
        return rd();
    });
    return ret;
}

int main()
{
    system("chcp 65001");

    std::set<int> s1;
    T s2;

    auto numbers1 = random_number();
    {
        ::leviathan::timer _;
        for (auto x : numbers1)
        s1.insert(x);
    }
    {
        ::leviathan::timer _;
        for (auto x : numbers1)
        {
            s2.insert(x);
        }
    }

    int cnt1 = 0;
    int cnt2 = 0;
    auto numbers2 = random_number();
    // assert(false);
    assert(std::ranges::is_sorted(s2.begin(), s2.end()));
    assert(std::ranges::equal(s1, s2));
    assert(cnt1 == cnt2);
    std::cout << cnt1 << ' ' << cnt2 << '\n';

    s2.erase(s2.begin(), s2.end());
    std::cout << s2.size() << " " << std::distance(s2.begin(), s2.end()) << '\n';
    std::cout << "Ok\n";
}





