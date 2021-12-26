#include <iostream>
#include <vector>
#include <random>
#include <lv_cpp/algorithm/sort.hpp>
#include <algorithm>
#include <ranges>
#include <bit>
#include <assert.h>


int main(int argc, char const *argv[])
{
    std::vector<int> vec = {3, 1, 2};
    leviathan::sort::detail::median_three(vec.begin(), vec.begin() + 1, vec.begin() + 2, std::ranges::less{});
    for (auto i : vec) std::cout << i << '\n';
    std::cout << "\nOK\n";
    // __STDCPP_DEFAULT_NEW_ALIGNMENT__
    
    return 0;
}
