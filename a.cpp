#include <cstddef>
#include <iostream>
#include <algorithm>
#include <assert.h>
#include <ranges>
#include <vector>


#include <lv_cpp/collections/py_dict.hpp>


int main()
{
    ::leviathan::collections::hash_table<int> ls;

    assert(ls.insert(3).second == true);
    assert(ls.insert(5).second == true);
    assert(ls.insert(3).second == false);
    assert(ls.insert(3).second == false);
    assert(ls.insert(5).second == false);
    assert(ls.insert(6).second == true);

    ls.insert(2);
    ls.insert(12);
    ls.insert(22);
    ls.insert(32);
    ls.insert(52);
    ls.insert(7);
    

    std::cout << ls.size() << '\n';
    std::ranges::copy(ls, std::ostream_iterator<int>{std::cout, " "});

    std::cout << std::boolalpha;

    // std::cout << (ls.insert)

}








