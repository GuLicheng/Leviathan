#include <iostream>
#include <string>
#include <lv_cpp/collections/internal/raw_hash_table.hpp>


    // alignas(slot_type) unsigned char raw[sizeof(slot_type)];
    // slot_type* slot = reinterpret_cast<slot_type*>(&raw);

int main()
{
    unsigned char raw[10];

    std::cout << reinterpret_cast<int*>(raw) << '\n';
    std::cout << reinterpret_cast<int*>(&raw) << '\n';

}





