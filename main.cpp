#include <iostream>
#include <ranges>
#include <functional>
#include <algorithm>
#include <vector>
#include <list>
#include <print>
#include <leviathan/ranges/action.hpp>

namespace action = leviathan::action;

int main(int argc, char const *argv[])
{
    auto v = std::vector{1, 2, 3};
    // auto a = std::vector{1, 2, 3} | action::min;

    auto print = [](const auto& x) 
    {
        std::cout << std::format("{}", x);
    };

    v | action::reverse | action::for_each(print);

    // std::cout << a << '\n';

}
