#pragma once

// #include "core.hpp"
#include <leviathan/extc++/math.hpp>

#include <utility>
#include <cstdint>
#include <array>
#include <type_traits>
#include <concepts>
#include <span>

namespace cpp::math::algebra
{

template <std::floating_point T, size_t Dimension>
struct vector
{
    // static_assert(std::is_floating_point_v<T>);
    static_assert(2 <= Dimension && Dimension <= 4, "Vector dimension must be between 2 and 4.");

    using value_type = T;

private:

    static constexpr auto indices = std::make_index_sequence<Dimension>();

    static constexpr T epsilon = static_cast<T>(1e-5);

    // Helper functions
    template <typename UnaryOp, typename I, size_t... Indices>
    static constexpr vector unary_operation_impl(UnaryOp fn, I first, std::index_sequence<Indices...>)
    {
        return { fn(first[Indices])... };
    }

    template <typename UnaryOp>
    static constexpr vector unary_operation(UnaryOp fn, const vector& v)
    {
        return unary_operation_impl(fn, v.data(), indices);
    }

    template <typename BinaryOp, typename I, size_t... Indices>
    static constexpr vector binary_operation_impl(BinaryOp fn, I first, I second, std::index_sequence<Indices...>)
    {
        return { fn(first[Indices], second[Indices])... };
    }

    template <typename BinaryOp, typename I, size_t... Indices>
    static constexpr vector binary_operation_impl(BinaryOp fn, I first, T scale, std::index_sequence<Indices...>)
    {
        return { fn(first[Indices], scale)... };
    }

    template <typename BinaryOp>
    static constexpr vector binary_operation(BinaryOp fn, const vector& v1, const vector& v2)
    {
        return binary_operation_impl(fn, v1.data(), v2.data(), indices);
    }

    template <typename BinaryOp>
    static constexpr vector binary_operation(BinaryOp fn, const vector& v, T scale)
    {
        return binary_operation_impl(fn, v.data(), scale, indices);
    }

    // ||x||_p = Sigma|x_i| ^ (1/p)
    template <typename Fn1, typename Fn2, typename I, size_t... Idx>
    static constexpr T Lp_norm(Fn1 fn1, Fn2 fn2, I first, I second, std::index_sequence<Idx...>)
    {
        return fn1(fn2(first[Idx] - second[Idx])...);
    }

    static constexpr T euclidean_distance(const vector& v1, const vector& v2)
    {
        return math::sqrt(Lp_norm(math::sum, math::square, v1.data(), v2.data(), indices));
    }
    
    static constexpr T manhattan_distance(const vector& v1, const vector& v2)
    {
        return Lp_norm(math::sum, math::abs, v1.data(), v2.data(), indices);
    }

    static constexpr T chebyshev_distance(const vector& v1, const vector& v2)
    {
        return Lp_norm(math::max, math::abs, v1.data(), v2.data(), indices);
    }

public:

    static constexpr const vector zero_vector = vector();

    // Binary operations
    constexpr bool operator==(const vector& rhs) const = default;

    constexpr bool equals(const vector& rhs, T eps = epsilon) const
    {
        if constexpr (std::integral<T>)
        {
            return this->operator==(rhs);
        }
        else
        {
            const auto d1 = this->data();
            const auto d2 = rhs.data();
            
            for (size_t i = 0; i < Dimension; ++i)
            {
                if (math::abs(d1[i] - d2[i]) > eps)
                {
                    return false;
                }
            } 
            return true;
        }
    }

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
    
    constexpr vector& operator-=(const vector& rhs) 
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
            return math::sum(first[Indices] * second[Indices]...);
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

    constexpr T operator[](size_t idx) const
    {
        return m_data[idx];
    }

    constexpr T& operator[](size_t idx) 
    {
        return m_data[idx];
    }

    static constexpr T dot_product(const vector& v1, const vector& v2)
    {
        return v1 | v2;
    }

    static constexpr auto cross_product(const vector& v1, const vector& v2)
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

    static constexpr vector max(const vector& v1, const vector& v2)
    {
        return binary_operation(math::max, v1, v2);
    }

    static constexpr vector min(const vector& v1, const vector& v2)
    {
        return binary_operation(math::min, v1, v2);
    }

    static constexpr vector abs(const vector& v)
    {
        return unary_operation(math::abs, v);
    }

    static constexpr vector sign_vector(const vector& v)
    {
        return unary_operation(math::sign, v);
    }

    static constexpr T distance(const vector& v1, const vector& v2)
    {
        return euclidean_distance(v1, v2);
    }

    static constexpr T size(const vector& v)
    {
        return math::sqrt(size_square(v));
    }

    static constexpr T size_square(const vector& v)
    {
        return Lp_norm(math::sum, math::square, v.data(), zero_vector.data(), indices);
    }

    static constexpr T length(const vector& v)
    {
        return size(v);
    }

    static constexpr T length_square(const vector& v)
    {
        return size_square(v);
    }

    static constexpr vector normalize(const vector& v)
    {
        return binary_operation(std::multiplies<T>(), v, static_cast<T>(1) / size(v));
    }

    static constexpr vector safe_normalize(const vector& v)
    {
        const auto len = size_square(v);
        return len > epsilon 
            ? binary_operation(std::multiplies<T>(), v, static_cast<T>(1) / math::sqrt(len))
            : zero_vector;
    }

    constexpr size_t hash_code() const
    {
        size_t seed = 0;

        for (size_t i = 0; i < Dimension; ++i)
        {
            seed ^= std::hash<T>()(m_data[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        
        return seed;
    }

    constexpr vector() = default;

    template <typename... Ts>
    constexpr vector(Ts... xs) : m_data{ static_cast<T>(xs)... } { }

private:

    std::array<T, Dimension> m_data;

};

template <typename T>
using vector2 = vector<T, 2>;

template <typename T>
using vector3 = vector<T, 3>;

template <typename T>
using vector4 = vector<T, 4>;

using vector2f = vector<float, 2>;
using vector3f = vector<float, 3>;
using vector4f = vector<float, 4>;
using vector2d = vector<double, 2>;
using vector3d = vector<double, 3>;
using vector4d = vector<double, 4>;

} // namespace cpp::math::algebra

namespace cpp::math
{

using cpp::math::algebra::vector;

}


#include <format>

template <typename T, size_t N, typename CharT>
struct std::formatter<cpp::math::vector<T, N>, CharT>
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return m_fmt.parse(ctx);
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(const cpp::math::vector<T, N>& v, FormatContext& ctx) const
    {
        auto it = ctx.out();
        const char* delimiter = "(";
        for (auto value : v.as_span())
        {
            *it++ = std::exchange(delimiter, ", ") ;
            it = m_fmt.format(value, ctx);
        }
        return *it++ = ")";
    }

    std::formatter<T, CharT> m_fmt;
};


template <typename T, size_t Dimension>
struct std::hash<cpp::math::vector<T, Dimension>>
{
    static constexpr size_t operator()(const cpp::math::vector<T, Dimension>& v) 
    {
        return v.hash_code();
    }
};
