#include <iostream>
#include <format>

#include <stdfloat>
#include <leviathan/math/vector.hpp>

template class leviathan::math::vector<float, 3>;

int main(int argc, char const *argv[])
{

    std::cout << leviathan::math::summation(1, 2, 3) << '\n';
    std::cout << leviathan::math::max_value(1, 2, 3) << '\n';
    std::cout << leviathan::math::min_value(1, 2, 3) << '\n';
    
    using FVector = leviathan::math::vector<float, 3>;
    std::cout << FVector::size(FVector(1, 1, 1)) << '\n';

    // std::float16_t
    // std::float128_t

    return 0;
}
