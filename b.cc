#include <iostream>
#include <lv_cpp/collections/internal/binary_search_tree.hpp>


int main()
{
    using namespace leviathan::collections;
    binary_set<int> s;
    s.insert(0);
    s.begin()++;
    for (auto value : s) std::cout << value << ' ';
    std::cout << "\nOver\n";
    return 0;
}






