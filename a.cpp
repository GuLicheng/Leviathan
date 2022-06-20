#include <iostream>
#include <lv_cpp/collections/internal/raw_hash_table.hpp>

using T = ::leviathan::collections::hash_set<int>;

int main()
{
    // T hs = { 1, 2, 3, 4, 5 };
    T hs;
    hs.contains(5ll);
}


