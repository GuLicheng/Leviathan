
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <algorithm>

#include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/ranges.hpp>

int main()
{
    std::string text = "  this   is  a sentence ";
    const auto total_size = text.size();
    const auto blanks = std::ranges::count(text, ' ');
    const auto each = 3;
    const auto rest = 1;
    std::string end_of_rang = "   ";
    auto result = text 
            | std::views::split(' ') 
            | std::views::filter(std::ranges::size) 
            | leviathan::ranges::join_with(' ')
            | leviathan::ranges::concat_with(end_of_rang);

    std::cout << '"';
    std::ranges::copy(result, std::ostream_iterator<char>{std::cout});
    std::cout << '"';
    // for (auto c : result2)
        // std::cout << c;
}

