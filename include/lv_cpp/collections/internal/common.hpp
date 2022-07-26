#pragma once

#include <type_traits>
#include <utility>

namespace leviathan::collections
{
    struct identity
    {
        template <typename T>
        using type = T;

        constexpr const auto& operator()(const auto& x)
        { return x; }
    };

    struct select1st
    {
        // for std::pair<const K, V>
        template <typename T>
        using type = std::remove_const_t<std::tuple_element_t<0, T>>;

        constexpr const auto& operator()(const auto& x)
        { return std::get<0>(x); }
    };

    namespace detail
    {

        // C++17 version
        // template <typename T, typename = void>
        // struct is_transparent : std::false_type { };
    
        // template <typename T>
        // struct is_transparent<T, std::void_t<typename T::is_transparent>> : std::true_type { };

        // C++20 simply use concept and require statement
        template <typename T> concept is_transparent = requires 
        { typename T::is_transparent; };

        // Non-deduced contexts
        // https://en.cppreference.com/w/cpp/language/template_argument_deduction
        // if IsTransparent is true, return K1, otherwise K2 
        template <bool IsTransparent>
        struct key_arg_helper 
        {
            template <typename K1, typename K2>
            using type = K1;
        };

        template <>
        struct key_arg_helper<false> 
        {
            template <typename K1, typename K2>
            using type = K2;
        };

        template <bool IsTransparent, class K1, class K2>
        using key_arg = typename key_arg_helper<IsTransparent>::template type<K1, K2>;
    
        // return true if Args is const T& or T&&
        template <typename T, typename... Args>
        struct emplace_helper
        {
        private:
            constexpr static auto is_same_as_key = [](){
                if constexpr (sizeof...(Args) != 1)
                    return false;
                else
                    return std::conjunction_v<std::is_same<T, std::remove_cvref_t<Args>>...>;
            }();
        public:
            constexpr static bool value = is_same_as_key;
        };
    }






}




