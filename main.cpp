#include <iostream>
#include <queue>
#include <ranges>
#include <algorithm>
#include <string_view>
#include <format>
#include <string>

#include "leviathan/algorithm/heap.hpp"

template <typename Fn>
void test1(Fn fn)
{
    std::vector<int> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    fn(vec.begin(), vec.end());

    for (auto i : vec)
    {
        std::cout << i << " ";
    }
    std::cout << std::format("   Is heap?: {}\n", std::is_heap(vec.begin(), vec.end()));
}

template <typename Fn>
void test2(Fn fn)
{
    std::vector<int> vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> vec2;

    // fn(vec.begin(), vec.end());
    for (auto i : vec)
    {
        vec2.push_back(i);
        fn(vec2.begin(), vec2.end());
    }

    for (auto i : vec2)
    {
        std::cout << i << " ";
    }
    std::cout << std::format("   Is heap?: {}\n", std::is_heap(vec2.begin(), vec2.end()));
}

template<class I = int*>
void print(std::string_view rem, I first = {}, I last = {},
           std::string_view term = "\n")
{
    for (std::cout << rem; first != last; ++first)
        std::cout << *first << ' ';
    std::cout << term;
}

template <typename Fn>
void test3(Fn fn)
{
    std::array v{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    // std::array v{ 2, 4, 6, 3, 9, 8, 5, 1, 9 };
    print("initially, v: ", v.cbegin(), v.cend());
 
    std::ranges::make_heap(v);
    print("make_heap, v: ", v.cbegin(), v.cend());
 
    print("convert heap into sorted array:");
    for (auto n {std::ssize(v)}; n >= 0; --n)
    {
        // std::ranges::pop_heap(v.begin(), v.begin() + n);
        fn(v.begin(), v.begin() + n);
        print("[ ", v.cbegin(), v.cbegin() + n, "]  ");
        print("[ ", v.cbegin() + n, v.cend(), "]\n");
    }
}

int main(int argc, char const *argv[])
{
    test2([](auto first, auto last) {
        leviathan::algorithm::nd_heap_fn<2>::push_heap(std::ranges::subrange(first, last));
    });

    test2([](auto first, auto last) {
        std::ranges::push_heap(std::ranges::subrange(first, last));
    });

    return 0;
}
