#include "linq.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <lv_cpp/io/console.hpp>

int main()
{
    using leviathan::linq::from;

{
        std::cout << "=======================\n";
    int arr[] = {1, 2, 3, 4, 5, 6};
    auto seq = from(arr).take_while([](int x) { return x < 3; })
             .ordered_by([](int x) { return -x; })
             .for_each([](auto x) { std::cout << x << std::endl;})
             ;
}


}