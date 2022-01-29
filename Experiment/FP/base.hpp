#pragma once

#include <ranges>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <numeric>
#include <lv_cpp/ranges/to.hpp>

template <typename... Ts>
void Println(Ts... x)
{
    (std::cout << ... << x) << '\n';
}

// For iterator that take a sentinel that default constructable
template <typename I>
auto MakeRange(I iter)
{
    return std::ranges::subrange(iter, I{});
}









