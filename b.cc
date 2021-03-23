#include <iostream>
#include <tuple>
#include <utility>
#include <functional>
#include <lv_cpp/io/console.hpp>

std::array arr{1, 2, 3};

int main()
{
    console::write_line("the paras is {0} and {1} and first is {1}", 1, arr);
}