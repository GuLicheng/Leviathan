#ifndef __STRING_OPT_HPP__
#define __STRING_OPT_HPP__

#include <string>
#include <string_view>
#include <cctype>
#include <algorithm>

namespace leviathan::string
{
    constexpr const char* blank = " \t\n\r";

    template <typename Char, typename Traits>
    constexpr std::basic_string_view<Char, Traits> trim(std::basic_string_view<Char, Traits> str, const Char* s = blank) 
    {
        const auto begin = str.find_first_not_of(s);
        if (begin == str.npos)
            return {};
        const auto last = str.find_last_not_of(s);
        return str.substr(begin, last - begin + 1);
    }
} // namespace leviathan::string


#endif