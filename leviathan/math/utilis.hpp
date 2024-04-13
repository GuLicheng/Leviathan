#pragma once

#include <cmath>
#include <numbers>
#include <functional>

namespace leviathan::math
{

inline constexpr struct 
{
    template <typename T, std::same_as<T>... Ts>
    constexpr static T operator()(T x, Ts... xs)
    {
        return (x + ... + xs);
    }
} summation; 

inline constexpr struct 
{
    template <typename T1, std::same_as<T1> T2, std::same_as<T1>... Ts>
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
    template <typename T1, std::same_as<T1> T2, std::same_as<T1>... Ts>
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
    template <typename T>
    constexpr static T operator()(T x)
    {
        return x * x;
    }
} square;

inline constexpr struct 
{
    template <typename T>
    constexpr static T operator()(T x)
    {
        return std::abs(x);
    }
} absolute_value;

inline constexpr struct 
{
    template <typename T>
    constexpr static T operator()(T x)
    {
        return std::sqrt(x);
    }
} sqrt;

inline constexpr struct 
{
    template <typename T>
    constexpr static T operator()(T x)
    {
        return std::signbit(x) 
            ? static_cast<T>(-1) 
            : static_cast<T>(1);
    }
} sign;

} // namespace leviathan::math
