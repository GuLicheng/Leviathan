#pragma once

#include <leviathan/meta/template_info.hpp>

#include <utility>
#include <concepts>
#include <type_traits>
#include <array>
#include <functional>
#include <cstdint>
#include <cmath>

#include <assert.h>

namespace Leviathan::Math
{

template <std::floating_point T, size_t N>    
struct TVector
{
    static_assert(N > 1);
protected:

    constexpr static auto Indices = std::make_index_sequence<N>();

    struct Summation 
    {
        template <typename... Us>
        constexpr static T operator()(Us... us) 
        {
            static_assert((std::is_same_v<T, Us> && ...));
            return (us + ...);
        }
    };

    struct MaxValue
    {
        constexpr static T operator()(T x) 
        {
            return x;
        }

        template <typename... Us>
        constexpr static T operator()(T x1, T x2, Us... xs) 
        {
            static_assert((std::is_same_v<T, Us> && ... && true));
            return x1 < x2 
                ? MaxValue::operator()(x1, xs...)
                : MaxValue::operator()(x2, xs...);
        }   
    };

    struct Square
    {
        constexpr static T operator()(T x)
        {
            return x * x;
        }
    };

    struct AbsoluteValue
    {
        constexpr static T operator()(T x)
        {
            return std::abs(x);
        }
    };

    template <size_t I, size_t... Idx>
    consteval static TVector BasisImpl(std::index_sequence<Idx...>)
    {
        return { (Idx == I ? T(1) : T(0))... };
    }

    template <typename UnaryOp, typename I, size_t... Idx>
    constexpr static TVector UnaryOperation(UnaryOp fn, I first, std::index_sequence<Idx...>)
    {
        return { fn(first[Idx])... };
    }

    template <typename BinaryOp, typename I, size_t... Idx>
    constexpr static TVector BinaryOperation(BinaryOp fn, I first, I second, std::index_sequence<Idx...>)
    {
        return { fn(first[Idx], second[Idx])... };
    }

    template <typename BinaryOp, typename I, size_t... Idx>
    constexpr static TVector BinaryOperation(BinaryOp fn, I first, T scale, std::index_sequence<Idx...>)
    {
        return { fn(first[Idx], scale)... };
    }

    template <typename Fn1, typename Fn2, typename I, size_t... Idx>
    constexpr static T LDistance(Fn1 fn1, Fn2 fn2, I first, I second, std::index_sequence<Idx...>)
    {
        return fn2(fn1(first[Idx] - second[Idx])...);
    }

    template <typename I>
    constexpr static T EuclideanDistance(I first, I second)
    {
        return std::sqrt(LDistance(Square(), Summation(), first, second, Indices));
    }

    template <typename I>
    constexpr static T ManhattanDistance(I first, I second)
    {
        return LDistance(AbsoluteValue(), Summation(), first, second, Indices);
    }

    template <typename I>
    constexpr static T ChebyshevDistance(I first, I second)
    {
        return LDistance(AbsoluteValue(), MaxValue(), first, second, Indices);
    }

public:

    using value_type = T;

    // Operators(vector and vector)
    constexpr bool operator==(const TVector& rhs) const = default;

    constexpr TVector operator+(const TVector& rhs) const
    {
        return BinaryOperation(std::plus<T>(), Data.begin(), rhs.Data.begin(), Indices);
    }

    constexpr TVector operator-(const TVector& rhs) const
    {
        return BinaryOperation(std::minus<T>(), Data.begin(), rhs.Data.begin(), Indices);
    }

    constexpr TVector operator-() const
    { 
        return UnaryOperation(std::negate<T>(), Data.begin(), Indices);
    }

    constexpr TVector operator*(const TVector& rhs) const
    {
        return BinaryOperation(std::multiplies<T>(), Data.begin(), rhs.Data.begin(), Indices);
    }

    constexpr TVector& operator+=(const TVector& rhs)
    {
        return *this = *this + rhs;    
    }

    constexpr TVector& operator-=(const TVector& rhs)
    {
        return *this = *this - rhs;    
    }

    constexpr TVector& operator*=(const TVector& rhs)
    {
        return *this = *this * rhs;    
    }

    // Operators(vector and scale)
    constexpr TVector operator*(T scale) const
    {
        return BinaryOperation(std::multiplies<T>(), Data.begin(), scale, Indices);
    }

    friend constexpr TVector operator*(T scale, const TVector& v)
    {
        return v * scale;
    }

    constexpr TVector operator/(T scale) const
    {
        return this->operator*((T)1 / scale);
    }

    constexpr TVector& operator/=(T scale) 
    {
        return *this = *this / scale;
    }

    // Static Methods
    constexpr static T Distance(const TVector& lhs, const TVector& rhs)
    {
        return EuclideanDistance(lhs.Data.begin(), rhs.Data.begin());
    }

    constexpr static TVector Normalize(const TVector& v)
    {
        return BinaryOperation(std::multiplies<T>(), v.Data.begin(), 1 / Length(v), Indices);
    }

    constexpr static T Length(const TVector& v)
    {
        return Distance(v, TVector());
    }

    template <size_t I>
    constexpr static TVector Basis()
    {
        return BasisImpl<I>(Indices);
    }

    // Constructors
    constexpr TVector() = default;

    template <std::same_as<T>... Us>
    constexpr TVector(Us... us) : Data{ us... } { }

    // Other Methods
    void show() const
    {
        const char* delimiter = "(";
        for (int i = 0; i < N; ++i)
        {
            std::cout << std::exchange(delimiter, ", ") << Data[i];
        }
        std::cout << ')';
    }

    std::array<T, N> Data;   
};  

} // namespace Math


#include <format>

template <typename T, size_t N>
struct std::formatter<Leviathan::Math::TVector<T, N>>
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) 
    { 
        return m_fmt.parse(ctx); 
    }

    template <typename FmtContext>
    typename FmtContext::iterator format(const Leviathan::Math::TVector<T, N>& v, FmtContext& ctx) const
    {
        auto it = ctx.out();
        const char* delimiter = "(";

        for (auto value : v.Data)
        {
            *it++ = std::exchange(delimiter, ", ") ;
            it = m_fmt.format(value, ctx);
        }
            
        return *it++ = ")";
    }

    std::formatter<T> m_fmt;
};