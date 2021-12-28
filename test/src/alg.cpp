#include <iostream>
#include <vector>
#include <random>
#include <lv_cpp/algorithm/sort.hpp>
#include <algorithm>
#include <ranges>
#include <assert.h>

auto random = []() {
    static std::random_device rd;
    return rd() % 1000'0000;
};

template <typename Sort>
void test(Sort s, const std::vector<int>& vec, std::string_view name)
{
    auto vec2 = vec;

    // less relationship
    s(vec2);
    assert(std::ranges::is_sorted(vec2));
    
    // greater relationship
    s(vec2, std::ranges::greater{});
    assert(std::ranges::is_sorted(vec2 | std::views::reverse));

    // test for stability
    std::vector<std::pair<int, int>> vec3;
    std::ranges::transform(vec, std::back_inserter(vec3), [cnt = 0](int x) mutable
    {
        return std::make_pair(x, cnt++);
    });
    s(vec3, {}, &std::pair<int, int>::first);

    auto iter = std::ranges::adjacent_find(vec3, [](const auto left, const auto right)
    {
        return left.first == right.first && left.second > right.second;
    });
    std::cout << name << " is stable sort? " << std::boolalpha << (iter == vec3.end()) << '\n';    
}

int main(int argc, char const *argv[])
{
    std::vector<int> vec;
    std::generate_n(std::back_inserter(vec), 100'000, random);

    test(leviathan::insertion_sort, vec, "insertion sort");
    test(leviathan::merge_sort, vec, "merge sort");
    test(leviathan::heap_sort, vec, "heap sort");
    test(leviathan::tim_sort, vec, "tim sort");
    test(leviathan::quick_sort, vec, "quick sort");
    test(leviathan::intro_sort, vec, "intro sort");

    std::cout << "OK\n";
    return 0;
}
