#pragma once

#include <cstddef>
#include <bit>
#include <memory>
// #include <lv_cpp/meta/concepts.hpp>
#include <assert.h>

namespace leviathan::collections
{

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
            using alloc_traits = typename std::allocator_traits<Alloc>::template rebind_traits<T>;
            using alloc_type = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
            alloc_type a{ alloc };
            alloc_traits::deallocate(a, p, n); 
        }

    }

	struct identity { };

	struct select1st { };

    template <typename T, typename Tag>
    struct mode;

    template <typename T>
    struct mode<T, identity>
    {
        using key_type = T;
        using value_type = T;

        template <typename U>
        constexpr static const auto& get(const U& x) noexcept
        { return x; } 
    };

    template <typename T>
    struct mode<T, select1st>
    {
        using key_type = std::tuple_element_t<0, T>;
        using mapped_type = std::tuple_element<1, T>;
        using value_type = std::pair<const key_type, mapped_type>;
        
        template <typename U>
        constexpr static const auto& get(const U& x) noexcept
        { return std::get<1>(x); } 
    };

    template <typename T>
    struct cache_hash_code : std::true_type { };

        template <typename T, bool Cache>
        struct storage_impl
        {
            T m_value;

            constexpr auto& value() noexcept 
            { return m_value; }

            constexpr auto& value() const noexcept
            { return m_value; }

            constexpr storage_impl(const T& x) : m_value { x } { }
            constexpr storage_impl(T&& x) : m_value { std::move(x) } { }

        };

        template <typename T>
        struct storage_impl<T, true> : storage_impl<T, false>
        {
            std::size_t m_hash_code;

            template <typename... Args>
            constexpr storage_impl(std::size_t hash_code, Args&&... args)
                : storage_impl<T, false>{ args... }, m_hash_code{ hash_code } { }
        };



    template <std::size_t Num = 2, std::size_t Den = 3, std::size_t PerturbShift = 5>
    struct py_hash_generator
    {
        constexpr py_hash_generator(std::size_t hash_init, std::size_t table_size) noexcept 
        {
            assert(is_power_of_two(table_size));
            m_mask = table_size - 1;
            m_perturb_shift = hash_init;
            m_value = hash_init & m_mask;
        }

        constexpr std::size_t operator()() noexcept
        {
            m_perturb_shift >>= PerturbShift;
            m_value = (5 * m_value + 1 + m_perturb_shift) & m_mask;
            return m_value;
        }

        constexpr static bool is_power_of_two(std::size_t x) noexcept
        { return std::popcount(x) == 1; }

        constexpr std::size_t operator*() const noexcept
        { return m_value; }

        std::size_t m_value;
        std::size_t m_perturb_shift;
        std::size_t m_mask;
    };

    template <typename T,
        typename HashGenerator,
        typename Mode,
        bool CacheHashCode,
        std::size_t MinSize, 
        std::size_t Num, 
        std::size_t Den>
    struct pydict_policy
    {

        using slot_type = storage_impl<T, CacheHashCode>;

        using key_type = typename mode<T, Mode>::key_type;
        
        using generator_type = HashGenerator;

        // template <typename U>
        // constexpr static const auto& get(const U& x) noexcept
        // // { return mode<T, Mode>::get(x); } 
        // { return x; } 

        constexpr static const auto& get(const key_type& x) noexcept
        { return x; }

        constexpr static double factor = (double) Num / Den;

        constexpr static bool need_expand(double load_factor) noexcept
        { return load_factor > factor; }

        constexpr static std::size_t next_capacity(std::size_t capacity) noexcept
        {
            return capacity << 1;
        }

        constexpr static bool is_power_of_two(std::size_t x) noexcept
        { return std::popcount(x) == 1; }

    };


    template <typename T>
    using default_hash_set_policy = pydict_policy<
        T, 
        py_hash_generator<>, 
        identity, 
        cache_hash_code<T>::value, 
        8, 2, 3>;

    template <typename K, typename V>
    using default_hash_map_policy = pydict_policy<
        std::pair<K, V>, 
        py_hash_generator<>, 
        select1st, 
        cache_hash_code<std::pair<K, V>>::value, 
        8, 2, 3>;

}


