#include <format>
#include <ranges>
#include <algorithm>
#include <vector>
#include <iostream>
#include <set>
#include <source_location>
#include <leviathan/collectionsV2/tree/avl_tree.hpp> 

using Hasher = std::hash<int>;
using KeyEqual = std::ranges::equal_to;

using MyEqualTo = leviathan::collections::hash_key_equal<Hasher, KeyEqual>;


int main(int argc, char const *argv[])
{

    leviathan::collections::avl_tree<int> t; 

    t.insert(1);
    t.insert(2);
    t.insert(0);

    auto walker = t.end();

    walker.left();

}
