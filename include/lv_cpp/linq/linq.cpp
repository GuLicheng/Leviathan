#include "linq.hpp"

#include <iostream>
#include <vector>


int main()
{
    using leviathan::linq::from;
    std::vector arr = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector arr2 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    leviathan::linq::from(arr)
                .reverse()
                .concat(from(arr2))
                .for_each([](auto x) { std::cout << x << ' '; })
                ;

    arr = {1, 1, 1, 1, 1, 1};
    // leviathan::linq::from(arr).distinct().for_each([](auto x){ std::cout << x << std::endl;});
}