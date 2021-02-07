#ifndef __STRING_OPT_HPP__
#define __STRING_OPT_HPP__

#include <string>
#include <string_view>
#include <cctype>
#include <algorithm>

namespace leviathan::string
{
    
    constexpr const char* blank = " \t\n";

    template <typename Char, typename Traits, typename Alloc>
    std::basic_string<Char, Traits, Alloc> trim(std::basic_string<Char, Traits, Alloc> const& str, const Char* s = blank)
    {   
        const auto begin = str.find_first_not_of(s);
        if (begin == str.npos)
            return {};
        const auto last = str.find_last_not_of(s);
        return str.substr(begin, last - begin + 1);
    }

    template <typename Char, typename Traits, typename Alloc>
    std::basic_string<Char, Traits, Alloc> trim(std::basic_string<Char, Traits, Alloc>&& str, const Char* s = blank)
    {   
        const auto begin = str.find_first_not_of(s);
        if (begin == str.npos)
            return {};
        const auto last = str.find_last_not_of(s);
        std::copy(str.begin() + begin, str.begin() + last + 1, str.begin());
        str.erase(last - begin + 1);
        return std::move(str);
    }

    template <typename Char, typename Traits>
    std::basic_string_view<Char, Traits> trim(std::basic_string_view<Char, Traits> const& str, const Char* s = blank) 
    {
        const auto begin = str.find_first_not_of(s);
        if (begin == str.npos)
            return {};
        const auto last = str.find_last_not_of(s);
        return str.substr(begin, last - begin + 1);
    }

} // namespace leviathan::string


#endif