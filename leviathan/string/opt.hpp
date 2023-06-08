#pragma once

#include <leviathan/meta/concepts.hpp>

#include <type_traits>
#include <string>
#include <string_view>
#include <algorithm>
#include <concepts>
#include <cstring>
#include <vector>
#include <mutex>

namespace leviathan::string
{

    // This overload participates in overload resolution only if 
    // Hash::is_transparent and KeyEqual::is_transparent are valid and each denotes a type
    // https://en.cppreference.com/w/cpp/container/unordered_map/find

    template <typename T>
    concept string_viewable = requires (const T& t) 
    {
        static_cast<std::string_view>(t);
    };

    struct string_hash_keyequal
    {
        using is_transparent = void;

        template <string_viewable Lhs, string_viewable Rhs>
        constexpr static bool operator()(const Lhs& l, const Rhs& r)  
        {
            std::string_view sl = static_cast<std::string_view>(l);
            std::string_view sr = static_cast<std::string_view>(r);
            return compare_impl(sl, sr);
        }

        template <string_viewable Str>
        constexpr static bool operator()(const Str& s)  
        {
            std::string_view sv = static_cast<std::string_view>(s);
            return hash_impl(sv);
        }

    private:

        constexpr static bool compare_impl(std::string_view lhs, std::string_view rhs)
        { return lhs == rhs; }  

        constexpr static size_t hash_impl(std::string_view sv)
        { return std::hash<std::string_view>()(sv); }

    };
 
    constexpr std::string_view trim(std::string_view sv)
    {
        auto left = sv.find_first_not_of(" \r\n\t");
        if (left == sv.npos) // all space
            return { sv.end(), sv.end() };
        auto right = sv.find_last_not_of(" \r\n\t");
        return { sv.begin() + left, sv.begin() + right + 1 };
    }

    std::string_view trim(const std::string& s)
    {
        std::string_view sv = s;
        return trim(sv);        
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

}