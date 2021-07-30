#define Line(x) (std::cout << x << '\n')
#include <lv_cpp/utils/test.hpp>
// #include "a.hpp"
#include <lv_cpp/collections/skip_list.hpp>
#include "l.hpp"
#include <lv_cpp/utils/timer.hpp>
#include <assert.h>
#include <set>
#include <lv_cpp/collections/List/SkipList.hpp>

auto random_num = []()
{
    return std::random_device()() % 50;
};
std::vector<int> vec;
leviathan::skip_list<int> ls;
::SkipList<int> LS;
std::set<int> s;
::Skiplist leet;

auto set_insert()
{
    leviathan::timer _;
    for (const auto& e : vec) 
        s.insert(e);
}

auto skip_list_insert()
{
    leviathan::timer _;
    for (const auto& e : vec) 
        ls.insert(e);
}

auto skip_list_insert2()
{
    leviathan::timer _;
    for (const auto& e : vec) 
        LS.insert(e);
}

auto skip_list_insert3()
{
    leviathan::timer _;
    for (const auto& e : vec) 
        leet.add(e);
}

int main()
{
    std::generate_n(std::back_inserter(vec), 100'0000, random_num);

    std::iota(vec.begin(), vec.end(), 0);
    std::reverse(vec.begin(), vec.end());
    // std::sort(vec.begin(), vec.end());
    // vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    // std::shuffle(vec.begin(), vec.end(), std::default_random_engine(std::random_device()()));
    set_insert();
    skip_list_insert();
    skip_list_insert2();
    skip_list_insert3();
    std::cout << (s.size() == ls.size() && s.size() == LS.size()) << std::endl;
}