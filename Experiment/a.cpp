#include <algorithm>
#include <iterator>
#include <ranges>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <random>
#include <lv_cpp/math/algorithm.hpp>
#include <lv_cpp/meta/function_traits.hpp>
#include <assert.h>


auto rand_number = []() {
    static std::random_device rd;
    return rd() % 100;
};

int main()
{
    std::vector<int> vec{ 5, 4, 3, 2, 1, 6, 5, 3, 2, 1, 4, 6, 78, 3, 3, 123, 4, 2, 534 };
    std::generate_n(std::back_inserter(vec), 100'000, rand_number);
    // tim_sort_fn::count_run_and_make_ascending(vec.begin(), vec.end(), std::ranges::less(), std::identity());
    leviathan::tim_sort(vec.begin(), vec.end());
    assert(std::ranges::is_sorted(vec));
    //std::ranges::copy(vec, std::ostream_iterator<int>{std::cout, " "});
    std::cout << "OK\n";
}
