#pragma once

#include <type_traits>
#include <utility>
#include <memory>

#include <assert.h>

namespace leviathan::collections
{
    /**
     * @brief A helper meta for set-container
     * 
     * Since set/map may implemented by same data structure, so we can 
     * add another template parameter to make the data structure set or map.
     * Different from some STL implementations, we don't use template class.
    */
    struct identity
    {
        /**
         * @brief Traits key_type from typelist.
         * @return T for set<T> and const T for set<const T>.
        */
        template <typename T> 
        using key_type = std::tuple_element_t<0, T>;

        /**
         * @brief Traits value_type from typelist.
         * @return T for set<T> and const T for set<const T>.
        */
        template <typename T> 
        using value_type = key_type<T>;

        // For set<T>, the key_type is value_type.
        template <typename T>
        constexpr auto&& operator()(T&& x) const 
        { return (T&&)x; }
    };

    /**
     * @brief A helper meta for map-container
     * 
     * Since set/map may implemented by same data structure, so we can 
     * add another template parameter to make the data structure set or map.
     * Different from some STL implementations, we don't use template class.
    */
    struct select1st
    {
        /**
         * @brief Traits key_type from typelist.
         * @return K for map<K, V> and const K for map<const K, V>.
        */
        template <typename T> 
        using key_type = std::tuple_element_t<0, T>;

        /**
         * @brief Traits value_type from typelist.
         * 
         * @return std::pair<const K, V> for map<K, V>.
         *  and std::pair<const K, V> for map<const K, V>.
        */
        template <typename T> 
        using value_type = std::pair<const std::tuple_element_t<0, T>, std::tuple_element_t<1, T>>;

        /**
         * @brief Get key_type from value_type.
         * @param x value_type
         * @return x.first
        */
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

        /**
         * @brief: A helper meta use non-deduced contexts to change function overload.
         * 
         * https://en.cppreference.com/w/cpp/language/template_argument_deduction
         * There are some overloads in lookup member functions.
         * E.g.
         * iterator find(const Key& key);
         * template <typename K> iterator find(const K& key);
         * This meta helper can reduce the number from 2 to 1.
         * 
         * template <typename U> using key_arg_t = detail::key_arg<IsTransparent, U, key_type>;
         * template <typename K = key_type> iterator find(const key_arg_t<K>& x);
         * 
         * @return: K1 if IsTransparent is true, otherwise K2.
        */
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
    
        /**
         * @brief: A helper meta for optimizing emplace function in some containers.
         * 
         * For some non-duplicated containers such as std::set, the emplace may first
         * construct a new value. However, if there is only one argument and the type 
         * is const T&/T&/const T&&/T&&, this step is not necessary. So this meta
         * is used to check whether the arguments passed in is followed type.
         * 
         * @return True if std::remove_cvref_t<Args> is same as T.
        */
        template <typename T, typename... Args>
        struct emplace_helper
        {
            constexpr static bool value = []() {
                if constexpr (sizeof...(Args) != 1)
                    return false;
                else
                    return std::conjunction_v<std::is_same<T, std::remove_cvref_t<Args>>...>;
            }();
        };
    }

    namespace detail
    {
        /**
         * @brief Allocate memory using allocator.
         * 
         * Automatically rebind Alloc to Alloc<T>.
         * @param T Target type.
         * @param alloc Allocator.
         * @param n number of elements.
         * @return Address of memory.
         * @exception Any exception that Alloc will throw.
        */
        template <typename T, typename Alloc>
        T* allocate(Alloc& alloc, std::size_t n)
        {
            using alloc_traits = typename std::allocator_traits<Alloc>::template rebind_traits<T>;
            using alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
            alloc_type a{ alloc };
            // The alloc_traits::allocate will return alloc_traits::pointer, please
            // make sure it can behavior as a raw pointer.
            T* addr = alloc_traits::allocate(a, n); 
            return addr;
        }

        /**
         * @brief Deallocate memory using allocator.
         * 
         * Automatically rebind Alloc to Alloc<T>.
         * @param alloc Allocator.
         * @param p Address that will be deallocated.
         * @param n number of elements.
        */
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



