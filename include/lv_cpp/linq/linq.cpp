#include "linq.hpp"

#include <iostream>

int main()
{
    int arr[] = {1, 2, 3};
    leviathan::linq::from(arr)
                .for_each([](auto x){ std::cout << x << std::endl;})
                ;
}