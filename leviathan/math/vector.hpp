#pragma once

#include "utilis.hpp"

#include <utility>
#include <cstdint>
#include <array>
#include <type_traits>
#include <concepts>
#include <span>

namespace leviathan::math
{

template <typename T, size_t Dimension>
struct vector
{
    static_assert(std::is_floating_point_v<T>);
    static_assert(2 <= Dimension && Dimension <= 5);

    using value_type = T;

private:

    constexpr static auto indices = std::make_index_sequence<Dimension>();

    constexpr static T small_number = static_cast<T>(1e-5);

    // Helper functions
    template <typename UnaryOp, typename I, size_t... Indices>
    constexpr static vector unary_operation_impl(UnaryOp fn, I first, std::index_sequence<Indices...>)
    {
        return { fn(first[Indices])... };
    }

    template <typename UnaryOp>
    constexpr static vector unary_operation(UnaryOp fn, const vector& v)
    {
        return unary_operation_impl(fn, v.data(), indices);
    }

    template <typename BinaryOp, typename I, size_t... Indices>
    constexpr static vector binary_operation_impl(BinaryOp fn, I first, I second, std::index_sequence<Indices...>)
    {
        return { fn(first[Indices], second[Indices])... };
    }

    template <typename BinaryOp, typename I, size_t... Indices>
    constexpr static vector binary_operation_impl(BinaryOp fn, I first, T scale, std::index_sequence<Indices...>)
    {
        return { fn(first[Indices], scale)... };
    }

    template <typename BinaryOp>
    constexpr static vector binary_operation(BinaryOp fn, const vector& v1, const vector& v2)
    {
        return binary_operation_impl(fn, v1.data(), v2.data(), indices);
    }

    template <typename BinaryOp>
    constexpr static vector binary_operation(BinaryOp fn, const vector& v, T scale)
    {
        return binary_operation_impl(fn, v.data(), scale, indices);
    }

    // ||x||_p = Sigma|x_i| ^ (1/p)
    template <typename Fn1, typename Fn2, typename I, size_t... Idx>
    constexpr static T Lp_norm(Fn1 fn1, Fn2 fn2, I first, I second, std::index_sequence<Idx...>)
    {
        return fn1(fn2(first[Idx] - second[Idx])...);
    }

    constexpr static T euclidean_distance(const vector& v1, const vector& v2)
    {
        return sqrt(Lp_norm(summation, square, v1.data(), v2.data(), indices));
    }
    
    constexpr static T manhattan_distance(const vector& v1, const vector& v2)
    {
        return Lp_norm(summation, absolute_value, v1.data(), v2.data(), indices);
    }

    constexpr static T chebyshev_distance(const vector& v1, const vector& v2)
    {
        return Lp_norm(max_value, absolute_value, v1.data(), v2.data(), indices);
    }

public:

    // Binary operations
    constexpr bool operator==(const vector& rhs) const = default;

    constexpr vector operator+(const vector& rhs) const
    {
        return binary_operation(std::plus<T>(), *this, rhs);
    }

    constexpr vector& operator+=(const vector& rhs) 
    {
        return *this = *this + rhs;
    }

    constexpr vector operator-(const vector& rhs) const
    {
        return binary_operation(std::minus<T>(), *this, rhs);
    }
    
    constexpr vector operator-=(const vector& rhs) 
    {
        return *this = *this - rhs;
    }

    constexpr vector operator*(const vector& rhs) const
    {
        return binary_operation(std::multiplies<T>(), *this, rhs);
    }

    constexpr vector& operator*=(const vector& rhs) 
    {
        return *this = *this * rhs;
    }

    constexpr vector operator*(T scale) const
    {
        return binary_operation(std::multiplies<T>(), *this, scale);
    }

    constexpr vector& operator*=(T scale) 
    {
        return *this = *this * scale;
    }

    friend constexpr vector operator*(T scale, const vector& v)
    {
        return v * scale;
    }

    constexpr vector operator/(const vector& rhs) const
    {
        return binary_operation(std::divides<T>(), *this, rhs);
    }

    constexpr vector& operator/=(const vector& rhs) 
    {
        return *this = *this / rhs;
    }

    constexpr vector operator/(T scale) const
    {
        return *this * (static_cast<T>(1) / scale);
    }

    constexpr vector& operator/=(T scale) 
    {
        return *this = *this / scale;
    }

    // Dot product
    constexpr T operator|(const vector& rhs) const
    {
        return []<typename I, size_t... Indices>(I first, I second, std::index_sequence<Indices...>)
        {
            return summation(first[Indices] * second[Indices]...);
        }(data(), rhs.data(), indices);
    }

    // Cross product
    constexpr auto operator^(const vector& rhs) const
    {
        if constexpr (Dimension == 2)
        {
            return m_data[0] * rhs.m_data[1] - m_data[1] * rhs.m_data[0];
        }
        else if constexpr (Dimension == 3)
        {
            return vector 
            {
                m_data[1] * rhs.m_data[2] - m_data[2] * rhs.m_data[1],
                m_data[2] * rhs.m_data[0] - m_data[0] * rhs.m_data[2],
                m_data[0] * rhs.m_data[1] - m_data[1] * rhs.m_data[0],
            };
        }
        else if constexpr (Dimension == 4)
        {
            return vector 
            { 
                m_data[1] * rhs.m_data[2] - m_data[2] * rhs.m_data[1], 
                m_data[2] * rhs.m_data[0] - m_data[0] * rhs.m_data[2],
                m_data[0] * rhs.m_data[1] - m_data[1] * rhs.m_data[0],
                T(0)
            };
        }
        else
        {
            std::unreachable();
        }
    }

    // Unary operation
    constexpr vector operator-() const
    {
        return unary_operation(std::negate<T>(), *this);
    }

    // constexpr bool equals(const vector& rhs, T eps = small_number) const
    // {

    // }

    constexpr T operator[](size_t idx) const
    {
        return m_data[idx];
    }

    constexpr T& operator[](size_t idx) 
    {
        return m_data[idx];
    }

    constexpr static T dot_product(const vector& v1, const vector& v2)
    {
        return v1 | v2;
    }

    constexpr static auto cross_product(const vector& v1, const vector& v2)
    {
        return v1 ^ v2;
    }

    constexpr T* data() 
    { 
        return m_data.data(); 
    }

    constexpr const T* data() const 
    { 
        return m_data.data(); 
    }

    std::span<T, Dimension> as_span()
    {
        return std::span<T, Dimension>(m_data.data(), m_data.size());
    }

    std::span<const T, Dimension> as_span() const
    {
        return std::span<const T, Dimension>(m_data.data(), m_data.size());
    }

    consteval static size_t dimension()
    {
        return Dimension;
    }

    constexpr vector() = default;

    template <typename... Ts>
        requires (sizeof...(Ts) == Dimension)
    constexpr vector(Ts... xs) : m_data{ static_cast<T>(xs)... } { }

private:
    std::array<T, Dimension> m_data;
};


} // namespace leviathan::math

