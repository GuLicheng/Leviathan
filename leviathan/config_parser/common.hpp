#pragma once

#include <leviathan/encode.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/string/fixed_string.hpp>
#include <leviathan/table.hpp>
#include "optional.hpp"

#include <iostream>
#include <array>
#include <fstream>
#include <string>
#include <cstdint>
#include <concepts>
#include <charconv>

#include <ctype.h>

namespace cpp::config
{
    
using cpp::read_file_context;
using cpp::encode_unicode_to_utf8;
using cpp::is_unicode;

using cpp::string::ltrim;
using cpp::string::rtrim;
using cpp::string::trim;
using cpp::string::replace;
using cpp::string::whitespace_delimiters;

}

namespace cpp::config
{

template <std::integral I>
constexpr optional<I> from_chars_to_optional(const char* startptr, const char* endptr, int base = 10)
{
    I value;
    auto result = std::from_chars(startptr, endptr, value, base);
    if (result.ec == std::errc() && result.ptr == endptr)
        return value;
    return nullopt;
}

template <std::floating_point F>
constexpr optional<F> from_chars_to_optional(const char* startptr, const char* endptr, std::chars_format fmt = std::chars_format::general)
{
    F value;
    auto result = std::from_chars(startptr, endptr, value, fmt);
    if (result.ec == std::errc() && result.ptr == endptr)
        return value;
    return nullopt;
}

template <typename T, typename... Args>
constexpr optional<T> from_chars_to_optional(const std::string& s, Args... args)
{
    return from_chars_to_optional<T>(
        s.data(), s.data() + s.size(), args...);
}

template <typename T, typename... Args>
constexpr optional<T> from_chars_to_optional(std::string_view sv, Args... args)
{
    return from_chars_to_optional<T>(
        sv.begin(), sv.end(), args...);
}
}

namespace cpp::config
{

using cpp::make_character_table;

namespace detail
{
    struct digit_value_config
    {
        constexpr int operator()(size_t x) const
        {
            if ('0' <= x && x <= '9') return x - '0';       // [0-9]
            if ('a' <= x && x <= 'f') return x - 'a' + 10;  // [a-f]
            if ('A' <= x && x <= 'F') return x - 'A' + 10;  // [A-F]
            return -1;
        }
    };

    inline constexpr auto digit_values = make_character_table(detail::digit_value_config());
}

/**
 * @brief Decode unicode from C-style string. 
 * 
 * @param p The string to be decoded, please make sure that the p has at least N bytes.
 * @param N Length of p, 4 for '\u' and 8 for '\U'.
*/
template <size_t N> 
constexpr unsigned decode_unicode_from_char(const char* p)
{
    static_assert(N == 2 || N == 4 || N == 8);

    auto to_digit = [](char ch) -> unsigned
    {
        // The static variable will make this function non-constexpr.
        return detail::digit_values[ch];
        // if (ch >= '0' && ch <= '9') return ch - '0';
        // if (ch >= 'a' && ch <= 'z') return ch - 'a' + 10;
        // return ch - 'A' + 10;
    };

    unsigned i = 0;

    if constexpr (N == 2)
    {
        i |= to_digit(p[0]) << 4;
        i |= to_digit(p[1]);
    }
    else if constexpr (N == 4)
    {
        i |= to_digit(p[0]) << 12;
        i |= to_digit(p[1]) << 8;
        i |= to_digit(p[2]) << 4;
        i |= to_digit(p[3]);
    }
    else
    {
        i |= to_digit(p[0]) << 28;
        i |= to_digit(p[1]) << 24;
        i |= to_digit(p[2]) << 20;
        i |= to_digit(p[3]) << 16;
        i |= to_digit(p[4]) << 12;
        i |= to_digit(p[5]) << 8;
        i |= to_digit(p[6]) << 4;
        i |= to_digit(p[7]);
    }
    return i;
}

template <typename Value>
struct value_parser;

// constexpr bool is_utf8_continuation_byte(unsigned char ch)
// {
//     return (ch & 0b11000000) == 0b10000000;
// }
}

namespace cpp::config::xml { }
namespace cpp::config::json { }
namespace cpp::config::toml { }
namespace cpp::config::cmd { }
namespace cpp::config::ini { }

namespace cpp
{

namespace json = config::json;
namespace toml = config::toml;
namespace xml = config::xml;
namespace cmd = config::cmd;
namespace ini = config::ini;

} // namespace cpp

