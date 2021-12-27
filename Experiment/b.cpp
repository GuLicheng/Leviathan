#include <iostream>
#include <vector>
#include <random>
#include <lv_cpp/algorithm/sort.hpp>
#include <algorithm>
#include <ranges>
#include <bit>
#include <assert.h>

struct RandomRange
{
    inline static std::random_device rd;

    int num = 1000'000;
    
    std::vector<int> RandomRangeInt()
    {
        auto random_generator = [&]() {
            return rd();
        };
        std::vector<int> ret;
        std::generate_n(std::back_inserter(ret), num, random_generator);
        return ret;
    }

    std::vector<int> RandomAscending()
    {
        auto ret = RandomRangeInt();
        std::sort(ret.begin(), ret.end());
        return ret;
    }

    std::vector<int> RandomDescending()
    {
        auto ret = RandomRangeInt();
        std::sort(ret.begin(), ret.end(), std::greater<>{});
        return ret;
    }

    std::vector<int> RandomPipeOrganInt()
    {
        auto ret = RandomRangeInt();
        auto iter = ret.begin() + ret.size() / 2;
        std::copy(ret.begin(), iter, iter + 1);
        std::reverse(iter, ret.end());
        return ret;
    }

};


int main(int argc, char const *argv[])
{
    std::vector<int> vec = {3, 1, 2};
    leviathan::sort::detail::median_three(vec.begin(), vec.begin() + 1, vec.begin() + 2, std::ranges::less{});
    for (auto i : vec) std::cout << i << '\n';
    std::cout << "\nOK\n";
    // __STDCPP_DEFAULT_NEW_ALIGNMENT__
    
    return 0;
}
