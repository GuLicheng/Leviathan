#include <regex>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <lv_cpp/algorithm/sort.hpp>

#include <assert.h>

auto random = []() {
    static std::random_device rd;
    return rd() % 1000'0000;
};

int main(int argc, char const *argv[])
{
    std::vector<int> vec1, vec2;
    std::generate_n(std::back_inserter(vec1), 100'000, random);

    vec2 = vec1;
    leviathan::insertion_sort(vec2);
    assert(std::ranges::is_sorted(vec2));

    vec2 = vec1;
    leviathan::merge_sort(vec2);
    assert(std::ranges::is_sorted(vec2));

    vec2 = vec1;
    leviathan::tim_sort(vec2);
    assert(std::ranges::is_sorted(vec2));

    std::cout << "OK\n";
    return 0;
}
