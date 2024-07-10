#pragma once

#include <charconv>
#include <concepts>
#include <optional>
#include <string_view>

namespace leviathan
{

template <typename T> struct parser;

template <std::integral T>
struct parser<T>
{
    static constexpr std::optional<T> operator()(std::string_view str, int base = 10)
    {
        T result;
        auto [ptr, ec] = std::from_chars(str.begin(), str.end(), result, base);
        return ec == std::errc() && ptr == str.end() 
             ? std::make_optional(result)
             : std::nullopt;
    }
};

template <std::floating_point T>
struct parser<T>
{
    static constexpr std::optional<T> operator()(std::string_view str, std::chars_format fmt = std::chars_format::general)
    {
        T result;
        auto [ptr, ec] = std::from_chars(str.begin(), str.end(), result, fmt);
        return ec == std::errc() && ptr == str.end() 
             ? std::make_optional(result)
             : std::nullopt;
    }
};

}
