#include <iostream>
#include <format>
#include <vector>
#include <leviathan/algorithm/tim_sort.hpp>
#include <leviathan/algorithm/timsort.hpp>
#include <chrono>
#include <algorithm>
#include <random>
#include <leviathan/time/timer.hpp>


int main(int argc, char const *argv[])
{   
    auto vec = std::vector<int>(10000000);
    std::generate(vec.begin(), vec.end(), std::mt19937(std::random_device()()));

    auto vec1 = vec;


    return 0;
}
