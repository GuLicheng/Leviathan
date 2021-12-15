#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <random>
#include <lv_cpp/math/algorithm.hpp>
#include <lv_cpp/meta/function_traits.hpp>
#include <lv_cpp/utils/timer.hpp>
#include <assert.h>


auto rand_number = []() {
    static std::random_device rd;
    return rd();
};

int main()
{
    std::vector<int> vec1;
    std::generate_n(std::back_inserter(vec1), 100'00'00, rand_number);
    std::vector<int> vec2 = vec1;
    {
        leviathan::timer _{"TimSort"};
        leviathan::tim_sort(vec1);
    }

    {
        leviathan::timer _{"STL Sort"};
        std::ranges::sort(vec2);
    }
    assert(vec1 == vec2);
    std::cout << "OK\n";
}
