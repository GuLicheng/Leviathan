#include <format>
#include <ranges>
#include <algorithm>
#include <vector>
#include <iostream>
#include <set>
#include <source_location>
#include <leviathan/collections/list/skiplist.hpp> 

#include <leviathan/meta/template_info.hpp>

using namespace leviathan::collections;

int main(int argc, char const *argv[])
{

    skiplist<identity<int>, std::ranges::less, std::allocator<int>, true> sl;

    std::mt19937 rd;

    std::default_random_engine Engine;

    for (auto i = 0; i < 10; ++i)
    {
        std::cout << rd() << '\n';
    } 

}
