#include <iostream>
#include <leviathan/extc++/all.hpp>

int main()
{
    std::ofstream writer("E:\\Library\\Leviathan\\flag2.jpg", std::ios::out | std::ios::binary);

    cpp::read_file_context("E:\\Library\\Leviathan\\flag.jpg")
    | std::views::chunk(4) 
    | std::views::transform(std::views::reverse) 
    | std::views::join
    | std::ranges::to<std::string>()
    | cpp::action::invoke(std::ranges::copy, cpp::console_iterator(writer, ""));

}
