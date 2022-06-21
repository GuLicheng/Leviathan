#include <iostream>
#include <string>
#include <lv_cpp/collections/internal/raw_hash_table.hpp>


    // alignas(slot_type) unsigned char raw[sizeof(slot_type)];
    // slot_type* slot = reinterpret_cast<slot_type*>(&raw);

int main()
{
    ::leviathan::collections::hash_map<int, int> h1, h2;
    std::swap(h1, h2);
}





