#pragma once

#include <type_traits>
#include <utility>
#include <memory>

#include <assert.h>

namespace leviathan::collections
{
    // For set<int>, the key_type and value_type is int
    // For set<const int>, the key_type and value_type is const int
    struct identity
    {
        template <typename T> 
        using key_type = std::tuple_element_t<0, T>;

        template <typename T> 
        using value_type = key_type<T>;

        template <typename T>
        constexpr auto&& operator()(T&& x) const 
        { return (T&&)x; }
    };

    // For map<int, int>, the key_type is int and value_type is std::pair<const int, int>
    // For map<const int>, the key_type is const int and value_type is std::pair<const int, int>
    struct select1st
    {
        template <typename T> 
        using key_type = std::tuple_element_t<0, T>;

        template <typename T> 
        using value_type = std::pair<const std::tuple_element_t<0, T>, std::tuple_element_t<1, T>>;

        template <typename T>
        constexpr auto&& operator()(T&& x) const 
        { return std::get<0>((T&&)x); }
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

    namespace detail
    {
        template <typename T, typename Alloc>
        T* allocate(Alloc& alloc, std::size_t n)
        {
            using alloc_traits = typename std::allocator_traits<Alloc>::template rebind_traits<T>;
            using alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
            alloc_type a{ alloc };
            T* addr = alloc_traits::allocate(a, n); 
            return addr;
        }

        template <typename Alloc, typename T>
        void deallocate(Alloc& alloc, T* p, std::size_t n)
        {
            assert(p && "p should not be nullptr");
            using alloc_traits = typename std::allocator_traits<Alloc>::template rebind_traits<T>;
            using alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
            alloc_type a{ alloc };
            alloc_traits::deallocate(a, p, n); 
        }
    }
}



