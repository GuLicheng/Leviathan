#include <array>
#include <iostream>
#include "../../test/utils/struct.hpp"

std::array<foo, 10> get_array(int x)
{
    std::array<foo, 10> arr;
    for (auto val : arr) val.val = x;
    return arr;
}

int main()
{
    // we can use array as return because it will copy anther one,
    // of course it is a waste !!!
    auto arr = get_array(2);
    std::cout << "Test Successfully\n";
}