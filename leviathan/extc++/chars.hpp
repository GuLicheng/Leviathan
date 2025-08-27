#pragma once

#include <cctype>
#include <string_view>

namespace cpp
{
    
inline constexpr struct 
{
    template <typename CharT>
    static constexpr bool operator()(CharT c)
    {
        using std::isxdigit;
        return isxdigit(c);
    }
} ishexdigit;

inline constexpr struct 
{
    template <typename CharT>
    static constexpr bool operator()(CharT c)
    {
        return c == CharT('0') || c == CharT('1');
    }
} isbindigit;

inline constexpr struct chars
{
    template <typename CharT>
    static constexpr bool operator()(CharT c)
    {
        return c >= CharT('0') && c <= CharT('7');
    }
} isoctdighit;


} // namespace cpp


