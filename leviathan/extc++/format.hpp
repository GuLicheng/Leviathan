#pragma once

#include <format>
#include <variant>
#include <ranges>
#include <concepts>
#include <utility>

template <typename... Ts, typename CharT>
struct std::formatter<std::variant<Ts...>, CharT>
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(const std::variant<Ts...>& v, FormatContext& ctx) const
    {
        return std::visit(
            [&](const auto& value) { 
                return std::format_to(ctx.out(), "{}", value);
            }, v
        );
    }
};

// Not support std::generator
template <std::ranges::range View, typename CharT>
struct std::formatter<View, CharT> 
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return m_fmt.parse(ctx);
    }

    template <typename V2, typename FormatContext>
    typename FormatContext::iterator format(V2&& v, FormatContext& ctx) const
    {
        auto it = ctx.out();
        const char* delimiter = "[";
        
        for (auto&& value : v)
        {
            *it++ = std::exchange(delimiter, ", ") ;
            it = m_fmt.format(value, ctx);
        }
        return *it++ = "]";
    }

    std::formatter<std::ranges::range_value_t<View>, CharT> m_fmt;
};

template <typename T1, typename T2, typename CharT>
struct std::formatter<std::pair<T1, T2>, CharT>
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(const std::pair<T1, T2>& v, FormatContext& ctx) const
    {
        auto it = ctx.out();
        *it++ = '(';
        *it++ = std::format("{}", v.first);
        *it++ = ",";
        *it++ = std::format("{}", v.second);
        *it++ = ')';
        return it;  
    }
};