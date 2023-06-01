#include "random_range.hpp"
#include <iostream>

int main(int argc, char const *argv[])
{
    auto strs = leviathan::random_range::random_long_string(10);
    for (const auto& str : strs)
        std::cout << str << '\n';
    return 0;
}
