#pragma once

#include <utility>
#include <format>
#include <cstdio>
#include <string>

template <std::ranges::range View, typename CharT>
struct std::formatter<View, CharT> 
{

    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return m_fmt.parse(ctx);
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(const View& v, FormatContext& ctx) const
    {
        auto it = ctx.out();
        const char* delimiter = "(";
        for (const auto& value : v)
        {
            *it++ = std::exchange(delimiter, ", ") ;
            it = m_fmt.format(value, ctx);
        }
        return *it++ = ")";
    }


    std::formatter<std::ranges::range_value_t<View>, CharT> m_fmt;
};


template <typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args)
{
    ::printf("%s", std::format(fmt, (Args&&) args...).c_str());
}

template <typename... Args>
void println(std::format_string<Args...> fmt, Args&&... args)
{
    ::puts(std::format(fmt, (Args&&) args...).c_str());
}

