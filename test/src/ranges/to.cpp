#include <lv_cpp/io/console.hpp>
#include <lv_cpp/ranges/to.hpp>

#include <bits/stdc++.h>

int main()
{
    auto allo = std::allocator<int>();
    std::vector arr{1, 2, 3};
    auto list1 = arr 
        | ::std::views::filter([](auto i) { return i & 1; }) 
        | ::std::views::reverse
        | ::std::views::transform([](auto i) { return i * 3; })
        | ::leviathan::views::to<std::list<int>>(std::allocator<int>());

    auto list2 = arr 
        | ::std::views::filter([](auto i) { return i & 1; }) 
        | ::std::views::reverse
        | ::std::views::transform([](auto i) { return i * 3; })
        | ::leviathan::views::to<std::list<int>>();
    console::write_line(list1);
    console::write_line(list2);
}