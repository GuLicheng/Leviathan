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

    sl.insert(9);
    sl.insert(1);
    sl.insert(7);
    sl.insert(3);
    sl.insert(5);

    sl.show();


    auto it1 = sl.lower_bound(1);
    auto it2 = sl.end();

    std::cout << std::format("sl.lower_bound(1) = {}\n", *it1);


}
