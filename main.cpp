#include <iostream>
#include <format>
#include <vector>
#include <leviathan/algorithm/tim_sort.hpp>
#include <leviathan/algorithm/timsort.hpp>
#include <chrono>
#include <algorithm>
#include <random>
#include <leviathan/time/timer.hpp>
#include <tim/timsort.h>

void TimSort1(auto& vec)
{
    leviathan::time::timer t;
    leviathan::algorithm::tim_sort(vec.begin(), vec.end());
}

void TimSort2(auto& vec)
{
    leviathan::time::timer t;
    // gfx::timsort(vec.begin(), vec.end());
    // std::stable_sort(vec.begin(), vec.end());
    tim::timsort(vec.begin(), vec.end());
}


int main(int argc, char const *argv[])
{   
    auto vec = std::vector<int>(10000000);
    std::generate(vec.begin(), vec.end(), std::mt19937(std::random_device()()));

    auto vec1 = vec;

    TimSort1(vec);
    TimSort2(vec1);

    return 0;
}
