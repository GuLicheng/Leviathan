#pragma once
#if 0
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

#endif













// #include <lv_cpp/enum.hpp>
#include <lv_cpp/meta/template_info.hpp>
#include <iostream>
#include <lv_cpp/string/fixed_string.hpp>
#include <lv_cpp/string/opt.hpp>
#include <concepts>
#include <ranges>
#include <compare>
#include <lv_cpp/format_extend.hpp>


namespace leviathan 
{


// Some complier-time functions
namespace detail
{

template <typename T>
constexpr T parser_hex(std::string_view sv)
{
    T x = 0;
    for (auto ch : sv)
    {
        if ('0' <= ch && ch <= '9') ch -= '0';
        else if ('a' <= ch && ch <= 'z') ch -= 'a' + 10;
        else if ('A' <= ch && ch <= 'Z') ch -= 'A' + 10;
        else;

        x = x << 4 | (T)ch;
    }
    return x;
}

template <typename T>
constexpr T parser_dec(std::string_view sv)
{
    T i = 0;
    if constexpr (std::signed_integral<T>)
    {
        bool sign = sv[0] == '-';
        if (sign) sv = sv.substr(1);
        for (const auto ch : sv)
        {
            i = i * (T)10 + (T)(ch - '0');
        }
        if (sign) i = -i;
    }
    else
    {
        for (const auto ch : sv)
        {
            i = i * (T)10 + (T)(ch - '0');
        }
    }
    return i;
}

template <std::integral Underlying>
constexpr Underlying parse_int(std::string_view sv)
{
    if (sv.starts_with("0x")) return parser_hex<Underlying>(sv.substr(2));
    else return parser_dec<Underlying>(sv);
    // assert sv is not empty

}

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

template <size_t N, basic_fixed_string FixedContext, typename UnderlyingType = int>
constexpr auto split_and_trait() 
{
    // return
    std::string_view context = FixedContext.sv();
    std::array<UnderlyingType, N> values;
    std::array<std::string_view, N> keys;
    size_t idx = 0;
    UnderlyingType value = 0;

    //    "Red , Green = 5 , Blue = 100";
    // context = trim(context)
    auto first = context.begin(), last = context.end();
    for (auto iter = first; ; idx++, value++)
    {
        auto next = std::find(iter, last, ',');

        std::string_view sub_context{ iter, next == last ? next - 1 : next };
        // std::cout << "Sub Context = " << sub_context << '\n';

        auto [k, v] = parse_key_value(sub_context);

        // std::cout << "Key = (" << k << ") Value = (" << v << ")\n";

        keys[idx] = trim(k);
        if (!v.empty())
            value = parse_int<UnderlyingType>(trim(v));
        values[idx] = value;
        // std::cout << "Value = " << value << '\n';
        if (next == last) break;
        else iter = next + 1;
        // std::cout << std::distance(first, iter);
    }
    using key_type = std::array<std::string_view, N>;
    using value_type = std::array<UnderlyingType, N>;
    return std::pair<key_type, value_type>{ keys, values };
}

} // namespace detail

template <typename Enum, basic_fixed_string info, bool Scope>
struct enum_list_base
{
    using enum_type = Enum;
    using underlying_type = std::underlying_type_t<enum_type>;
    constexpr static bool is_scope = Scope;
    constexpr static auto context = info;
    constexpr static auto enum_size = [](){ return std::ranges::count(info.data, info.data + info.size(), ',')  + 1; }();
    constexpr static auto kvs = detail::split_and_trait<enum_size, info>();

    constexpr static std::string_view search_name(Enum e)
    {
        const auto index = std::find(kvs.second.begin(), kvs.second.end(), static_cast<underlying_type>(e)) - kvs.second.begin();
        return kvs.first[index];
    }

    constexpr static auto keys() { return kvs.first; }
    constexpr static auto values() { return kvs.second; }

private:
    struct iterator_impl
    {
        size_t idx;
        using value_type = std::pair<std::string_view, underlying_type>;
        using reference = value_type;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;


        constexpr iterator_impl() = default;
        constexpr iterator_impl(size_t i) : idx{i} { }

        constexpr auto operator<=>(const iterator_impl&) const noexcept = default;
        constexpr reference operator*() const noexcept { return std::make_pair(kvs.first[idx], kvs.second[idx]); }
        constexpr reference operator[](difference_type n) const noexcept { return *(*this + n); }

        constexpr auto& operator++() { idx++; return *this; }
        constexpr auto operator++(int) { auto old = *this; ++*this; return old; }
        constexpr auto& operator--() { idx--; return *this; }
        constexpr auto operator--(int) { auto old = *this; --*this; return old; }
        
        constexpr auto& operator+=(difference_type n) { idx += n; return *this; }
        constexpr auto& operator-=(difference_type n) { idx -= n; return *this; }
        constexpr auto operator+(difference_type n) const { auto old = *this; old += n; return old; }
        constexpr auto operator-(difference_type n) const { auto old = *this; old -= n; return old; }
        constexpr difference_type operator-(iterator_impl rhs) const { return idx - rhs.idx; }
        friend constexpr auto operator+(difference_type n, iterator_impl rhs) { return rhs + n; }


    };
public:
    using iterator = iterator_impl;
    using const_iterator = iterator_impl;

    iterator begin() { return { 0 }; }
    const_iterator begin() const { return { 0 }; }
    iterator end() { return { enum_size }; }
    const_iterator end() const { return { enum_size }; }

};

template <typename Enum>
struct enum_list;


} // namespace leviathan

#define Enum(enumname, ...) \
    enum enumname { __VA_ARGS__ }; \
    template <> struct leviathan::enum_list<enumname> : leviathan::enum_list_base<enumname, #__VA_ARGS__ , false> { }

#define ScopeEnum(enumname, ...) \
    enum struct enumname { __VA_ARGS__ }; \
    template <> struct leviathan::enum_list<enumname> : leviathan::enum_list_base<enumname, #__VA_ARGS__ , true> { }

#define EnumWithUnderlying(enumname, underlying, ...) \
    enum enumname : underlying { __VA_ARGS__ }; \
    template <> struct leviathan::enum_list<enumname> : leviathan::enum_list_base<enumname, #__VA_ARGS__ , false> { }

#define ScopeEnumWithUnderlying(enumname, underlying, ...) \
    enum struct enumname : underlying { __VA_ARGS__ }; \
    template <> struct leviathan::enum_list<enumname> : leviathan::enum_list_base<enumname, #__VA_ARGS__ , true> { }

template <typename Enum, typename CharT, typename Traits> requires (std::is_enum_v<Enum>)
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, Enum e)  
{   
    using T = leviathan::enum_list<Enum>;
    if constexpr (T::is_scope)
        os << TypeInfo(Enum) << "::";
    return os << T::search_name(e);
}

