#pragma once

#include <lv_cpp/meta/concepts.hpp>

#include <type_traits>
#include <string>
#include <string_view>
#include <algorithm>
#include <concepts>
#include <cstring>
#include <vector>
#include <mutex>

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
                LV_STATIC_ASSERT_FALSE("no match");
        }
    };

    template <typename Target, typename Source>
    Target lexical_cast(const Source& source)
    {
        return lexical_cast_t<Target, Source>::cast(source);
    }

    // use for construct some exception infomation
    template <typename T, typename... Ts>
    [[deprecated("using lazy_string_concat_helper instead")]]
    std::string cat_string(const T& t, const Ts&... ts)
    {
        return (lexical_cast<std::string>(t) + ... + lexical_cast<std::string>(ts));
    }


    namespace detail
    {

        template <typename LValue, typename RValue>
        struct storage_base
        {
            using lvalue_type = LValue;
            using rvalue_type = RValue;
        };

        template <typename T>
        struct storage : storage_base<T, T>
        {
        };

        template <typename CharT, typename Traits, typename Alloc>
        struct storage<std::basic_string<CharT, Traits, Alloc>> 
            : storage_base<std::basic_string_view<CharT, Traits>, std::basic_string<CharT, Traits, Alloc>>
        {
        };

        template <>
        struct storage<const char*> : storage<std::string_view> { };

        template <typename StringType>
        struct straits;

        /*
            size_t length -> return length of string
            operator string_view -> convert to string_view
            operator string -> convert to string
        */

        template <typename CharT, typename Traits, typename Alloc>
        struct straits<std::basic_string<CharT, Traits, Alloc>> 
        {

            using string_type = std::basic_string<CharT, Traits, Alloc>;

            constexpr static size_t length(const string_type& s)
            { return s.size(); }
        
            constexpr std::basic_string_view<CharT, Traits> view(const string_type& s)
            { return s; }
        
            static std::basic_string<CharT> to_string(const string_type& s)
            { return s; }

            static std::basic_string<CharT> to_string(string_type&& s)
            { return std::move(s); }

        };

        template <typename CharT, typename Traits>
        struct straits<std::basic_string_view<CharT, Traits>>
        {

            using string_type = std::basic_string_view<CharT, Traits>;

            constexpr static size_t length(string_type s)
            { return s.size(); }
        
            constexpr std::basic_string_view<CharT, Traits> view(string_type s)
            { return s; }
        
            static std::basic_string<CharT> to_string(string_type s)
            { return s; }

        };

        template <>
        struct straits<const char*>
        {

            using string_type = const char*;

            constexpr static size_t length(string_type s)
            { return static_cast<size_t>(std::strlen(s)); }
        
            constexpr static std::string_view view(string_type s)
            { return s; }

            static std::string to_string(string_type s)
            { return s; }
        
        };

        template <typename String>
        struct lazy_string_map
        {
            using T = std::decay_t<String>;
            using type = std::conditional_t< 
                std::is_lvalue_reference_v<String>,
                typename storage<T>::lvalue_type,
                typename storage<T>::rvalue_type
            >;
        };

    }

    // for lvalue, this class will store string_view
    // for rvalue, only std::string will be moved into this class 
    // for user-defind string type, please specialize storage
    template <typename... Strings>
    struct lazy_string_concat_helper
    {

        static_assert((std::is_same_v<std::decay_t<Strings>, Strings> && ...), "Only Support decay type");

        std::tuple<Strings...> m_strs;

        mutable std::once_flag m_flag;
        mutable std::string m_cache;
        
    public:

        lazy_string_concat_helper(Strings... s) : m_strs { std::move(s)... } { }
        lazy_string_concat_helper(std::tuple<Strings...>&& s) : m_strs { std::move(s) } { }

        operator const std::string &() const
        {
            std::call_once(m_flag, [this]() { this->concat(); });
            return m_cache;
        }

        const std::string& to_string() const 
        { return this->operator const std::string &(); }

        // Retrun a new concat helper which will copy all elements in tuples
        template <typename String>
        lazy_string_concat_helper<Strings..., typename detail::lazy_string_map<String>::type> cat_with(String&& s) const
        {
            std::tuple<typename detail::lazy_string_map<String>::type> tail{ (String&&)s };
            return { std::tuple_cat(m_strs, std::move(tail)) };            
        }


    private:
        void concat() const 
        {
            // calculate length
            auto len = std::apply([]<typename... T>(const T&... t) {
                return (std::size(t) + ... + 0);
            }, m_strs);

            m_cache.reserve(len);
            
            // concat
            std::apply([&, this]<typename... T>(const T&... t) {
                ((m_cache += t), ...);
            }, m_strs);
        }

    };

    template <typename... Strings>
    lazy_string_concat_helper(Strings&&...) -> 
        lazy_string_concat_helper<typename detail::lazy_string_map<Strings>::type...>;

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


}