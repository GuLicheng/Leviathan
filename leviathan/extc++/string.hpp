#pragma once

#include <format>
#include <string>
#include <string_view>
#include <ranges>
#include <concepts>

namespace leviathan::string
{
    
template <typename T>
concept string_viewable = std::is_convertible_v<const T&, std::string_view>;

template <typename T>
concept string_view_like = string_viewable<T> && !std::is_convertible_v<const T&, const char*>;

// This overload participates in overload resolution only if 
// Hash::is_transparent and KeyEqual::is_transparent are valid and each denotes a type
// https://en.cppreference.com/w/cpp/container/unordered_map/find
struct string_hash_key_equal
{
    using is_transparent = void;

    template <string_viewable Lhs, string_viewable Rhs>
    constexpr static bool operator()(const Lhs& l, const Rhs& r) 
    {
        std::string_view sl = static_cast<std::string_view>(l);
        std::string_view sr = static_cast<std::string_view>(r);
        return key_equal_impl(sl, sr);
    }

    template <string_viewable Str>
    constexpr static bool operator()(const Str& s) 
    {
        std::string_view sv = static_cast<std::string_view>(s);
        return hash_impl(sv);
    }

private:

    static constexpr bool key_equal_impl(std::string_view lhs, std::string_view rhs)
    {
        return lhs == rhs;
    }

    static constexpr size_t hash_impl(std::string_view sv)
    {
        return std::hash<std::string_view>()(sv);
    }
};

/**
 * ' '  space    
 * '\t' horizontal tab
 * '\r' carriage return
 * '\n' linefeed
*/
inline constexpr const char* whitespace_delimiters = " \t\n\r";

constexpr std::string_view ltrim(std::string_view sv, std::string_view delimiters = whitespace_delimiters)
{
    auto left = sv.find_first_not_of(delimiters);
    return sv.substr(left == sv.npos ? sv.size() : left);
}

constexpr std::string_view rtrim(std::string_view sv, std::string_view delimiters = whitespace_delimiters)
{
    auto right = sv.find_last_not_of(delimiters);
    return sv.substr(0, right + 1);
}

constexpr std::string_view trim(std::string_view sv, std::string_view delimiters = whitespace_delimiters)
{
    return rtrim(ltrim(sv, delimiters), delimiters);
}

template <string_viewable Str>
constexpr std::string_view trim(const Str& s, std::string_view delimiters = whitespace_delimiters)
{
    std::string_view sv = s;
    return trim(sv, delimiters);        
}

template <typename Pred>
constexpr std::string_view drop_while(std::string_view sv, Pred pred)
{
    auto iter = sv.begin(), sentinel = sv.end();
    for (; iter != sentinel && pred(*iter); ++iter);
    return { iter, sentinel };
}

template <typename Pred>
constexpr std::string_view take_while(std::string_view sv, Pred pred)
{
    auto iter = sv.begin(), sentinel = sv.end();
    for (; iter != sentinel && pred(*iter); ++iter);
    return { sv.begin(), iter };
}

inline std::string replace(std::string str, std::string_view from, std::string_view to)
{
    if (from == to)
    {
        return str;
    }

    size_t pos = 0;
    while ((pos = str.find(from, pos)) != str.npos)
    {
        str.replace(pos, from.size(), to);
        pos += to.size();
    }
    return str;
}

inline std::string replace(std::string str, char from, char to)
{
    if (from == to)
    {
        return str;
    }

    for (auto& ch : str)
    {
        if (ch == from)
        {
            ch = to;
        }
    }
    return str;
}

template <typename StrSequence, typename Delimiter>
std::string join(StrSequence&& str, Delimiter&& delimiter)
{
    return (StrSequence&&)str | std::views::join((Delimiter&&)delimiter) | std::ranges::to<std::string>();
}

inline std::string repeat(std::string_view str, size_t n)
{
    if (str.empty() || n == 0)
    {
        return "";
    }

    std::string ret;
    ret.reserve(str.size() * n);

    for (size_t i = 0; i < n; ++i)
    {
        ret += str;
    }

    return ret;
}

inline std::string center(const std::string& text, int width, char filler_character = ' ')
{
    if ((int)text.size() >= width)
    {
        return text;
    }

    const int length = (int)text.size();
    const int right = (width - length) / 2;
    const int left = width - length - right;

    return std::string(left, filler_character) + text + std::string(right, filler_character);
}

} // namespace leviathan::string

