#pragma once

#include <type_traits>
#include <string>
#include <string_view>
#include <algorithm>

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

}