#include <lv_cpp/collections/sorted_list.hpp>
#include <random>
#include <ranges>
#include <iostream>
#include <iterator>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>
#include <assert.h>
// A meta helper for select map/set

constexpr auto N = 30'00;
static std::random_device rd;

int main()
{
    sorted_list<int> ls;
    ls.insert(1);
    ls.insert(2);
    ls.insert(3);
    ls.insert(4);


    // ls.erase(ls.begin());
    // ls.erase(ls.begin());
    // ls.erase(ls.begin());
    // auto i = ls.erase(ls.begin());
    // std::cout << std::boolalpha << (i == ls.end()) << '\n'; 
    
    int cnt = 0;
    auto iter = ls.begin();
    while (iter != ls.end())
    {
        std::cout << "cnt = " << cnt++ << ", value = " << iter.m_in_idx << ',' << iter.m_out_idx << '\n';
        iter = ls.erase(iter);
    }

    for (auto val : ls) std::cout << val << ' ';

    std::cout << "\nOver\n";
}


