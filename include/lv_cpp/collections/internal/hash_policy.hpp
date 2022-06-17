#pragma once

#include <cstddef>
#include <bit>
#include <memory>
// #include <lv_cpp/meta/concepts.hpp>
#include <algorithm>
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



    template <std::size_t PerturbShift = 5>
    struct py_hash_generator
    {

        inline static int count = 0;

        constexpr py_hash_generator(std::size_t hash_init, std::size_t table_size) noexcept 
        {
            assert(is_power_of_two(table_size));
            m_mask = table_size - 1;
            m_perturb_shift = hash_init;
            m_value = hash_init & m_mask;
        }

        constexpr std::size_t operator()() noexcept
        {
            count++;
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

    template <std::size_t Offset = 1>
    struct linear_hash_generator
    {
        constexpr linear_hash_generator(std::size_t hash_init, std::size_t table_size) noexcept 
        {
            m_mask = table_size;
            m_value = hash_init % table_size;
        }

        constexpr std::size_t operator()() noexcept
        {
            m_value = (m_value + Offset) % m_mask;
            return m_value;
        }

        constexpr std::size_t operator*() const noexcept
        { return m_value; }

        std::size_t m_value;
        std::size_t m_mask;
    };

    struct quadratic_policy
    {
        quadratic_policy(std::size_t hash_init, std::size_t mask) noexcept 
        {
            assert(is_prime(mask));
            m_value = hash_init % mask;
            m_mask = mask;
            m_delta = 1;                
        }

        constexpr std::size_t operator()() noexcept
        {
            m_value += m_delta;
            m_delta += 2;
            m_value %= m_mask;
            return m_value;
        }

        constexpr std::size_t first() const noexcept
        { return m_value; }

    private:

        static inline std::size_t prime_table[] = {
            17, 29, 37, 59, 89, 127, 193, 293,
            433, 653, 977, 1459, 2203, 3307, 4967,
            7451, 11173, 16759, 25147, 37747, 56629,
            84947, 127423, 191137, 286711, 430081,
            645131, 967693, 1451539, 2177321,
            3265981, 4898981, 7348469, 11022709,
            16534069, 24801109, 37201663, 55802497, 83703749,
            125555621, 188333437, 282500161, 423750241, 635625377, 953438137
        };

        constexpr static bool is_prime(std::size_t x) noexcept
        { return std::ranges::find(prime_table, x) != std::ranges::end(prime_table); } 

        std::size_t m_value;
        std::size_t m_mask;
        std::size_t m_delta;
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


