#pragma once

#include "../common.hpp"

namespace leviathan::collections
{
    
template <typename Slot, bool CacheHashCode>
struct hash_cell;

template <typename Slot>
struct hash_cell<Slot, false>
{
    Slot m_value;

    constexpr auto& value()  
    { return m_value; }

    constexpr auto& value() const 
    { return m_value; }

    constexpr Slot* value_ptr() 
    { return std::addressof(m_value); }

    constexpr const Slot* value_ptr() const
    { return std::addressof(m_value); }
};

template <typename Slot>
struct hash_cell<Slot, true> : hash_cell<Slot, false>
{
    std::size_t m_hash_code;

    template <typename... Args>
    constexpr hash_cell(std::size_t hash_code, Args&&... args)
        : hash_cell<Slot, false>((Args&&) args...), m_hash_code{ hash_code } { }
};

namespace detail
{
    
template <typename T> 
struct cache_hash_code : std::true_type { };

template <std::size_t PerturbShift = 5>
struct py_hash_generator
{
    constexpr py_hash_generator(std::size_t hash_init, std::size_t table_size)  
    {
        assert(is_power_of_two(table_size));
        m_mask = table_size - 1;
        m_perturb_shift = hash_init;
        m_value = hash_init & m_mask;
    }

    constexpr std::size_t operator()() 
    {
        // count++;
        m_perturb_shift >>= PerturbShift;
        m_value = (5 * m_value + 1 + m_perturb_shift) & m_mask;
        return m_value;
    }

    static constexpr bool is_power_of_two(std::size_t x) 
    { return std::popcount(x) == 1; }

    constexpr std::size_t operator*() const 
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

// struct quadratic_policy
// {
//     quadratic_policy(std::size_t hash_init, std::size_t mask) noexcept 
//     {
//         assert(is_prime(mask));
//         m_value = hash_init % mask;
//         m_mask = mask;
//         m_delta = 1;                
//     }

//     constexpr std::size_t operator()() noexcept
//     {
//         m_value += m_delta;
//         m_delta += 2;
//         m_value %= m_mask;
//         return m_value;
//     }

//     constexpr std::size_t first() const noexcept
//     {
//         return m_value;
//     }

// private:

//     static inline std::size_t prime_table[] = {
//         17, 29, 37, 59, 89, 127, 193, 293,
//         433, 653, 977, 1459, 2203, 3307, 4967,
//         7451, 11173, 16759, 25147, 37747, 56629,
//         84947, 127423, 191137, 286711, 430081,
//         645131, 967693, 1451539, 2177321,
//         3265981, 4898981, 7348469, 11022709,
//         16534069, 24801109, 37201663, 55802497, 83703749,
//         125555621, 188333437, 282500161, 423750241, 635625377, 953438137
//     };

//     static constexpr bool is_prime(std::size_t x) noexcept
//     {
//         return std::ranges::find(prime_table, x) != std::ranges::end(prime_table);
//     }

//     std::size_t m_value;
//     std::size_t m_mask;
//     std::size_t m_delta;
// };


} // namespace detail

} // namespace leviathan::collections

