#include <lv_cpp/algorithm/core.hpp>

#include <iostream>

int main()
{
    std::vector<int> v = { 0, 1, 2, 3, 4 }, dest;
    auto [ret, _] = leviathan::pair_wise_transform(v, std::back_inserter(dest), std::plus<>());
    // assert(dest == { 1, 5 });
    for (auto val : dest)  
        std::cout << val << '\n';

}








