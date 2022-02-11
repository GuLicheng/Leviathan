// #include <lv_cpp/enum.hpp>
#include <lv_cpp/meta/template_info.hpp>
#include <iostream>
#include <lv_cpp/string/fixed_string.hpp>
#include <lv_cpp/string/opt.hpp>
#include <concepts>
#include <ranges>
#include <lv_cpp/format_extend.hpp>


using leviathan::basic_fixed_string;
using leviathan::trim;

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

    static void show()
    {
        auto& [k, v] = kvs;
        for (auto _ : k) std::cout << _ << ' ';
        std::endl(std::cout);
        for (auto _ : v) std::cout << _ << ' ';
        std::endl(std::cout);
    }

};

template <typename Enum>
struct enum_list;

#define Enum(enumname, ...) \
    enum enumname { __VA_ARGS__ }; \
    template <> struct enum_list<enumname> : enum_list_base<enumname, #__VA_ARGS__ , false> { }

#define ScopeEnum(enumname, ...) \
    enum struct enumname { __VA_ARGS__ }; \
    template <> struct enum_list<enumname> : enum_list_base<enumname, #__VA_ARGS__ , true> { }

#define EnumWithUnderlying(enumname, underlying, ...) \
    enum enumname : underlying { __VA_ARGS__ }; \
    template <> struct enum_list<enumname> : enum_list_base<enumname, #__VA_ARGS__ , false> { }

#define ScopeEnumWithUnderlying(enumname, underlying, ...) \
    enum struct enumname : underlying { __VA_ARGS__ }; \
    template <> struct enum_list<enumname> : enum_list_base<enumname, #__VA_ARGS__ , true> { }

template <typename Enum, typename CharT, typename Traits> requires (std::is_enum_v<Enum>)
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, Enum e)  
{   
    using T = enum_list<Enum>;
    if constexpr (T::is_scope)
        os << TypeInfo(Enum) << "::";
    return os << T::search_name(e);
}

Enum(Color, Red , Green = -5, Blue = -100);

ScopeEnumWithUnderlying(Sex, int, 
    Male = 0x000000, Female = 0x1111, Unknown = 0x0001);

int main(int c, char**v)
{
    std::cout << Blue << '\n';
    std::cout << Green << '\n';
    std::cout << Red << '\n';

    std::cout << Sex::Female << '\n';
    std::cout << Sex::Male << '\n';
    std::cout << Sex::Unknown << '\n';

    return 0;
}