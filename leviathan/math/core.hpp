#pragma once

#include <cmath>
#include <numbers>
#include <functional>
#include <concepts>
#include <compare>
#include <cinttypes>

#include <type_traits>
#include <bit>

// Any component should in sub-namespace.
namespace leviathan::math
{

inline constexpr struct 
{
    template <typename T, std::same_as<T>... Ts>
    static constexpr T operator()(T x, Ts... xs)
    {
        return (x + ... + xs);
    }
} sum; 

inline constexpr struct 
{
    template <typename T1, std::same_as<T1> T2, std::same_as<T1>... Ts>
    static constexpr T1 operator()(T1 x1, T2 x2, Ts... xs)
    {
        if constexpr (sizeof...(Ts) == 0)
        {
            return x1 < x2 ? x2 : x1;
        }
        else
        {
            return x1 < x2  
                ? operator()(x2, xs...)
                : operator()(x1, xs...);
        }
    }
} max;

inline constexpr struct 
{
    template <typename T1, std::same_as<T1> T2, std::same_as<T1>... Ts>
    static constexpr T1 operator()(T1 x1, T2 x2, Ts... xs)
    {
        if constexpr (sizeof...(Ts) == 0)
        {
            return x1 > x2 ? x2 : x1;
        }
        else
        {
            return x1 > x2  
                ? operator()(x2, xs...)
                : operator()(x1, xs...);
        }
    }
} min;

inline constexpr struct
{
    template <typename T>
    static constexpr T operator()(T x)
    {
        return x * x;
    }
} square;

inline constexpr struct 
{
    template <typename T>
    static constexpr T operator()(T x)
    {
        using std::abs;
        return abs(x);
    }
} abs;

inline constexpr struct 
{
    template <typename T>
    static constexpr T operator()(T x)
    {
        using std::sqrt;
        return sqrt(x);
    }
} sqrt; 

inline constexpr struct 
{
    template <typename T>
    static constexpr bool operator()(T x)
    {
        using std::signbit;
        return signbit(x);
    }
} signbit;

inline constexpr struct
{
    template <typename T>
    static constexpr int operator()(T x)
    {
        if (x == 0)
        {
            return 0;
        }
        return signbit(x) ? -1 : 1;
    }
} sign; 

inline constexpr struct 
{
    template <typename T>
    static constexpr int operator()(T x)
    {
        using std::popcount;
        return popcount(x);
    }
} popcount;

inline constexpr struct 
{
    template <typename T>
    static constexpr bool operator()(T x)
    {
        using std::has_single_bit;
        return has_single_bit(x);
    }
} has_single_bit;

inline constexpr struct 
{
    template <typename T>
    static constexpr int operator()(T x)
    {
        using std::countl_zero;
        return countl_zero(x);
    }
} countl_zero;

inline constexpr struct 
{
    template <typename T>
    static constexpr int operator()(T x)
    {
        using std::countr_zero;
        return countr_zero(x);
    }
} countr_zero;

inline constexpr struct 
{
    template <typename T>
    static constexpr int operator()(T x)
    {
        using std::countl_one;
        return countl_one(x);
    }
} countl_one;

inline constexpr struct 
{
    template <typename T>
    static constexpr int operator()(T x)
    {
        using std::countr_one;
        return countr_one(x);
    }
} countr_one;

// https://en.cppreference.com/w/cpp/numeric/math/div
// The order of quot and rem in std::div_t or other div-return types is not certainly.
template <typename T>
struct div_result
{
    T quotient;
    T remainder;

    constexpr div_result(T quot, T rem) : quotient(quot), remainder(rem) { }
    constexpr div_result(std::div_t x) : quotient(x.quot), remainder(x.rem) { }
    constexpr div_result(std::ldiv_t x) : quotient(x.quot), remainder(x.rem) { }
    constexpr div_result(std::lldiv_t x) : quotient(x.quot), remainder(x.rem) { }
    constexpr div_result(std::imaxdiv_t x) : quotient(x.quot), remainder(x.rem) { }
};

template <typename... Ts>
size_t hash_combine(const Ts&... ts)
{
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0814r0.pdf
    constexpr auto hash_combine_impl = []<typename T>(size_t& seed, const T& value)
    {
        seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    };

    size_t seed = 0;
    (hash_combine_impl(seed, ts), ...);
    return seed;
}

} // namespace leviathan::math