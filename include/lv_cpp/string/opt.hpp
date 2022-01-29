#pragma once

#include <lv_cpp/meta/concepts.hpp>

#include <type_traits>
#include <string>
#include <string_view>
#include <algorithm>
#include <concepts>

namespace leviathan
{

    // This overload participates in overload resolution only if 
    // Hash::is_transparent and KeyEqual::is_transparent are valid and each denotes a type
    // https://en.cppreference.com/w/cpp/container/unordered_map/find

    struct string_hash
    {
        using hash_type = std::hash<std::string_view>;
        using is_transparent = void;
    
        // size_t operator()(const char* str) const        { return hash_type{}(str); }
        // size_t operator()(std::string_view str) const   { return hash_type{}(str); }
        // size_t operator()(std::string const& str) const { return hash_type{}(str); }
        template <typename String>
        constexpr size_t operator()(const String& str) const 
        { 
            return hash_type{}(str);
        }
    };
 
    struct string_key_equal
    {
        using is_transparent = void;
        template <typename Lhs, typename Rhs>
        constexpr bool operator()(const Lhs& l, const Rhs& r) const 
        {
            return std::ranges::lexicographical_compare(l, r, std::equal_to<>());
        }
    };

    struct string_compare : string_key_equal
    {
    };

    template <template <typename...> typename HashSet>
    using string_hashset = HashSet<std::string, string_hash,  string_key_equal>;

    template <template <typename...> typename HashSet, typename Value>
    using string_hashmap = HashSet<std::string, Value, string_hash, string_key_equal>;

    template <typename Target, typename Source>
    struct lexical_cast_t;

    template <typename Source>
    struct lexical_cast_t<std::string, Source>
    {
        static std::string cast(const Source& source)
        {
            if constexpr (std::is_arithmetic_v<Source>)
                return std::to_string(source);
            else if constexpr (std::is_constructible_v<std::string, Source>)
                return std::string(source);
            else
                LV_STATIC_ASSERT("no match");
        }
    };

    template <typename Target, typename Source>
    Target lexical_cast(const Source& source)
    {
        return lexical_cast_t<Target, Source>::cast(source);
    }

    // use for construct some exception infomation
    template <typename T, typename... Ts>
    std::string cat_string(const T& t, const Ts&... ts)
    {
        return (lexical_cast<std::string>(t) + ... + lexical_cast<std::string>(ts));
    }

}