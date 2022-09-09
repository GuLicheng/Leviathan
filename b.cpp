#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <algorithm>
#include <ranges>


int main()
{
    std::string text = "  this   is  a sentence ";
    const auto total_size = text.size();
    const auto blanks = std::ranges::count(text, ' ');
    const auto each = 3;
    const auto rest = 1;
    auto result = text 
            | std::views::split(' ') 
            | std::views::filter(std::ranges::size) 
            | std::views::join_with(std::views::repeat(' ', each))
            | std::views::concat_with(std::views::repeat(' ', rest))
            | std::views::to<std::string>();
}

