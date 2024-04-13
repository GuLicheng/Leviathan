#include <iostream>
#include <format>
#include <leviathan/math/vector.hpp>

template class leviathan::math::vector<float, 3>;

int main(int argc, char const *argv[])
{

    std::cout << leviathan::math::summation(1, 2, 3) << '\n';
    std::cout << leviathan::math::max_value(1, 2, 3) << '\n';
    std::cout << leviathan::math::min_value(1, 2, 3) << '\n';

    return 0;
}
