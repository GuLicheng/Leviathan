#include <memory_resource>
#include <iostream>

#include "except_allocator.hpp"
#include <unordered_set>
#include <list>
#include <forward_list>
#include <set>

#include <lv_cpp/collections/internal/raw_hash_table.hpp>

int main()
{
    ::leviathan::collections::hash_set<
        int, 
        std::hash<::leviathan::collections::auto_hash>, 
        std::equal_to<>, 
        RecordPolymorphicAllocator<int>> hs;
    hs.insert(1);
    Report();
}

