#include <iostream>
#include <utility>

#include <lv_cpp/ranges/ranges.hpp>
#include <unordered_set>
#include <set>
#include <format>

int main(int argc, char const *argv[])
{

    int i = 0;
    const int* pa = &i;
    const int* pb = &i;
    
    auto c = pa - pb;
    return 0;
}
