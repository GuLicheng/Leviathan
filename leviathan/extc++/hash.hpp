#pragma once

#include "concepts.hpp"

namespace cpp::hash
{
    
inline constexpr struct
{
    template <typename... Ts>
    static constexpr size_t operator()(const Ts&... ts)
    {
        // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0814r0.pdf
        auto hash_combine_impl = []<typename T>(size_t& seed, const T& value) static
        {
            seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };
    
        size_t seed = 0;
        (hash_combine_impl(seed, ts), ...);
        return seed;
    }
} hash_combine;

template <meta::tuple_like TupleLike>
constexpr size_t hash_tuple_like(const TupleLike& tuple)
{
    return std::apply(hash_combine, tuple);
}

} // namespace cpp::hash

