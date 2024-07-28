#pragma once

#include <charconv>
#include <concepts>
#include <optional>
#include <string_view>

namespace leviathan
{

template <typename T, typename CharT = char> struct parser;

template <std::integral T>
struct parser<T, char>
{
    template <typename ParseContext>
    static constexpr std::optional<T> operator()(const ParseContext& ctx, int base = 10)
    {
        T result;
        auto [ptr, ec] = std::from_chars(ctx.data(), ctx.data() + ctx.size(), result, base);
        return ec == std::errc() && ptr == ctx.end() 
             ? std::make_optional(result)
             : std::nullopt;
    }
};

template <std::floating_point T>
struct parser<T, char>
{
    template <typename ParseContext>
    static constexpr std::optional<T> operator()(const ParseContext& ctx, std::chars_format fmt = std::chars_format::general)
    {
        T result;
        auto [ptr, ec] = std::from_chars(ctx.data(), ctx.data() + ctx.size(), result, fmt);
        return ec == std::errc() && ptr == ctx.end() 
             ? std::make_optional(result)
             : std::nullopt;
    }
};

}
