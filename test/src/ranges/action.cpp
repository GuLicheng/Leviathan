#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <lv_cpp/ranges/action.hpp>

int main()
{
    auto printer = [](auto x) { std::cout << x << ' '; };
    std::vector arr{ 1, 2, 3, 1, 2, 3, 5 };
    std::cout << "\n============================\n";
    arr | std::views::all | leviathan::action::sort();
    arr | leviathan::action::for_each(printer);
    std::cout << "\n============================\n";
    auto [iter, last] = arr | std::views::all | leviathan::action::unique();
    arr | leviathan::action::for_each(printer);
    std::cout << "\n============================\n";
    arr.erase(iter, last);
    arr | leviathan::action::for_each(printer);
    std::cout << "\n============================\n";
}