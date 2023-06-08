#pragma once

#include <optional>
#include <cstdint>
#include <string>
#include <concepts>
#include <type_traits>
#include <charconv>

#include "fixed_string.hpp"

#include <string.h>

namespace leviathan::string
{
    template <typename Target, typename Source>
    struct lexical_cast_t;

    template <typename Target, typename Source, typename... Args>
    auto lexical_cast(const Source& source, Args&&... args)
    { return lexical_cast_t<Target, Source>()(source, (Args&&) args...); }

    template <typename T>
    concept arithmetic = std::is_arithmetic_v<T>;

    // Only for std::basic_string with [CharT = char, Traits = ...]
    template <std::integral I>
    struct lexical_cast_t<I, std::string_view>
    {
        constexpr static std::optional<I> operator()(std::string_view s, int base = 10) 
        {
            if (I value; std::from_chars(s.begin(), s.end(), value).ec == std::errc())
                return value;
            return std::nullopt;
        }
    };

    template <std::floating_point F>
    struct lexical_cast_t<F, std::string_view>
    {
        constexpr static std::optional<F> operator()(std::string_view s, std::chars_format fmt = std::chars_format::general) 
        {
            if (F value; std::from_chars(s.begin(), s.end(), value, fmt).ec == std::errc())
                return value;
            return std::nullopt;
        }
    };

    template <arithmetic Target>
    struct lexical_cast_t<Target, std::string>
    {
        template <typename... Args>
        constexpr static auto operator()(const std::string& s, Args... args) 
        {
            return lexical_cast_t<Target, std::string_view>()(s, args...);
        }
    };

    template <arithmetic Target, size_t N>
    struct lexical_cast_t<Target, fixed_string<N>> 
    {
        template <typename... Args>
        constexpr static auto operator()(const fixed_string<N>& s, Args... args) 
        {
            return lexical_cast_t<Target, std::string_view>()(s.sv(), args...);
        }
    };

} // namespace leviathan

