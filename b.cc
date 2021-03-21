#include <iostream>
#include <tuple>
#include <utility>
#include <functional>
#include <lv_cpp/io/console.hpp>

std::array arr{1, 2, 3};

int main()
{
    // std::cout << format("the paras is {0} and {1} and first is {0}", 1, 2.0);
    console::format("the paras is {0} and {1} and first is {0}", 1, arr);
}