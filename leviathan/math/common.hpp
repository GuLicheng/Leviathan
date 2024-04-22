#pragma once

#include <cmath>
#include <numbers>
#include <functional>
#include <concepts>
#include <type_traits>
#include <bit>

namespace leviathan::math
{

// Signed integral 
template <typename T>
struct is_user_defined_signed_integral : std::false_type { };

template <typename T>
struct is_user_defined_signed_integral<const T> : is_user_defined_signed_integral<T> { };

template <typename T>
struct is_user_defined_signed_integral<volatile T> : is_user_defined_signed_integral<T> { };

template <typename T>
struct is_user_defined_signed_integral<const volatile T> : is_user_defined_signed_integral<T> { };

template <typename T>
concept signed_integral = std::signed_integral<T> || is_user_defined_signed_integral<T>::value;

// Unsigned integral
template <typename T>
struct is_user_defined_unsigned_integral : std::false_type { };

template <typename T>
struct is_user_defined_unsigned_integral<const T> : is_user_defined_unsigned_integral<T> { };

template <typename T>
struct is_user_defined_unsigned_integral<volatile T> : is_user_defined_unsigned_integral<T> { };

template <typename T>
struct is_user_defined_unsigned_integral<const volatile T> : is_user_defined_unsigned_integral<T> { };

template <typename T>
concept unsigned_integral = std::unsigned_integral<T> || is_user_defined_unsigned_integral<T>::value;

template <typename T>
concept integral = signed_integral<T> || unsigned_integral<T>;

// Floating point
template <typename T>
struct is_user_defined_floating_point : std::false_type { };

template <typename T>
struct is_user_defined_floating_point<const T> : is_user_defined_floating_point<T> { };

template <typename T>
struct is_user_defined_floating_point<volatile T> : is_user_defined_floating_point<T> { };

template <typename T>
struct is_user_defined_floating_point<const volatile T> : is_user_defined_floating_point<T> { };

template <typename T>
concept floating_point = std::floating_point<T> || is_user_defined_floating_point<T>::value;

template <typename T>
concept arithmetic = floating_point<T> || integral<T>;

inline constexpr struct 
{
    template <arithmetic T, std::same_as<T>... Ts>
    constexpr static T operator()(T x, Ts... xs)
    {
        return (x + ... + xs);
    }
} summation; 

inline constexpr struct 
{
    template <arithmetic T1, std::same_as<T1> T2, std::same_as<T1>... Ts>
    constexpr static T1 operator()(T1 x1, T2 x2, Ts... xs)
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
} max_value;

inline constexpr struct 
{
    template <arithmetic T1, std::same_as<T1> T2, std::same_as<T1>... Ts>
    constexpr static T1 operator()(T1 x1, T2 x2, Ts... xs)
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
} min_value;

inline constexpr struct
{
    template <arithmetic T>
    constexpr static T operator()(T x)
    {
        return x * x;
    }
} square;

inline constexpr struct 
{
    template <arithmetic T>
    constexpr static T operator()(T x)
    {
        using std::abs;
        return abs(x);
    }
} absolute_value;

inline constexpr struct 
{
    template <arithmetic T>
    constexpr static T operator()(T x)
    {
        using std::sqrt;
        return sqrt(x);
    }
} square_root; 

inline constexpr struct 
{
    template <arithmetic T>
    constexpr static T operator()(T x)
    {
        using std::signbit;
        return signbit(x) 
            ? static_cast<T>(-1) 
            : static_cast<T>(1);
    }
} sign;

} // namespace leviathan::math
