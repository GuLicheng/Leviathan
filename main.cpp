#include <format>
#include <ranges>
#include <algorithm>
#include <vector>
#include <iostream>
#include <set>

#include <leviathan/collectionsV2/common.hpp> 

using Hasher = std::hash<int>;
using KeyEqual = std::ranges::equal_to;

using MyEqualTo = leviathan::collections::hash_key_equal<Hasher, KeyEqual>;

int main(int argc, char const *argv[])
{
    MyEqualTo a;

    std::cout << a(1) << '\n'; 
    std::cout << a(1, 1) << '\n'; 

}
