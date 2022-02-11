#pragma once

/*
    This header file supply some macro to create printable enum
    
    Examples:
        1. Enum(Color, Red, Blue = 1, Green) 
            equivalent to:  enum Color { Red, Blue = 1, Green }
            cout << Red; // -> Red


        2. ScopedEnumWithUnderlying(Sex, int, Male, Female)
            equivalent to:  enum class Color : int { Red, Blue = 1, Green }
            cout << Male; // -> Sex::Male
*/

#include <iostream>
#include <string>
#include <vector>
#include <string_view> 
#include <utility> 

#include <lv_cpp/meta/template_info.hpp> 
#include <lv_cpp/string/opt.hpp>

namespace leviathan
{

// http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
std::vector<std::string> split(const std::string &text, char sep) {
    std::vector<std::string> tokens;
    int start = 0, end = 0;
    while ((end = text.find(sep, start)) != std::string::npos) {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(text.substr(start));
    return tokens;
}

// parser such as `key = value`
constexpr std::pair<std::string_view, std::string_view> parse_key_value(std::string_view kv)
{
    // key = value
    kv = trim(kv);
    const auto equal_pos = kv.find_first_of('=');
    if (equal_pos == kv.npos)
        return { kv, "" };
    else
    {
        std::string_view key = { kv.begin(), kv.begin() + equal_pos };
        std::string_view value = { kv.begin() + equal_pos + 1, kv.end() };
        key = trim(key);
        value = trim(value);
        return { key, value };
    }
}

template <typename Underlying = int>
std::pair<std::vector<std::string>, std::vector<Underlying>>
split_and_trait(const std::string &text, char sep) {
    auto tokens = split(text, sep);
    std::vector<Underlying> indices;
    Underlying index = 0;
    for (auto& name : tokens)
    {
        auto [k, v] = parse_key_value(name);
        name = k;
        if (v.size()) 
            index = std::stoi(std::string(v));
        indices.emplace_back(index);
        index++;
    }
    return { tokens, indices };
}

} // namespace leviathan

#define Enum(name, ...) enum name { __VA_ARGS__ }; \
template <typename CharT, typename Traits> \
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, name e)  \
{   using namespace leviathan; \
    static auto [name##names, name##indices] = split_and_trait(#__VA_ARGS__, ',');\
    /* auto[names, indices] = ... -> gcc: Error: symbol `_ZNDC5names7indicesEE' is already defined */ \
    auto& names = name##names; \
    auto& indices = name##indices; \
    using underlying = std::underlying_type_t<name>; \
    const auto index = std::find(indices.begin(), indices.end(), static_cast<underlying>(e)) - indices.begin(); \
    return os << names[index];\
}

#define EnumWithUnderlying(name, int_type, ...) enum name : int_type { __VA_ARGS__ }; \
template <typename CharT, typename Traits> \
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, name e)  \
{   using namespace leviathan; \
    static auto [name##names, name##indices] = split_and_trait(#__VA_ARGS__, ',');\
    /* auto[names, indices] = ... -> gcc: Error: symbol `_ZNDC5names7indicesEE' is already defined */ \
    auto& names = name##names; \
    auto& indices = name##indices; \
    using underlying = std::underlying_type_t<name>; \
    const auto index = std::find(indices.begin(), indices.end(), static_cast<underlying>(e)) - indices.begin(); \
    return os << names[index];\
}


#define ScopedEnum(name, ...) enum struct name { __VA_ARGS__ }; \
template <typename CharT, typename Traits> \
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, name e)  \
{   using namespace leviathan; \
    static auto [name##names, name##indices] = split_and_trait(#__VA_ARGS__, ',');\
    auto& names = name##names; \
    auto& indices = name##indices; \
    using underlying = std::underlying_type_t<name>; \
    const auto index = std::find(indices.begin(), indices.end(), static_cast<underlying>(e)) - indices.begin(); \
    return os << std::string(TypeInfo(name)) + "::" + names[index];\
}

#define ScopedEnumWithUnderlying(name, int_type, ...) enum name : int_type { __VA_ARGS__ }; \
template <typename CharT, typename Traits> \
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, name e)  \
{   using namespace leviathan; \
    static auto [name##names, name##indices] = split_and_trait(#__VA_ARGS__, ',');\
    auto& names = name##names; \
    auto& indices = name##indices; \
    using underlying = std::underlying_type_t<name>; \
    const auto index = std::find(indices.begin(), indices.end(), static_cast<underlying>(e)) - indices.begin(); \
    return os << std::string(TypeInfo(name)) + "::" + names[index];\
}


/* test.cpp

Enum(Color, Red , Green = 5, Blue = 100)
ScopedEnumWithUnderlying(Sex, size_t, Male = 5, Female, Unknown)

int main(int c, char**v)
{
    std::cout << Blue << '\n';
    std::cout << Green << '\n';
    std::cout << Red << '\n';

    std::cout << Sex::Female << '\n';
    std::cout << Sex::Male << '\n';

    return 0;
}
*/