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
    static_assert(1 < N && N < 5);
protected:

    constexpr static auto Indices = std::make_index_sequence<N>();

    constexpr static T SmallNumber = (T)1e-5;

    // Helper classes
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
                ? MaxValue::operator()(x2, xs...)
                : MaxValue::operator()(x1, xs...);
        }   
    };

    struct MinValue
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
                ? MinValue::operator()(x1, xs...)
                : MinValue::operator()(x2, xs...);
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

    struct Sign
    {
        constexpr static T operator()(T x)
        {
            return std::signbit(x) ? (T)-1 : (T)1;
        }
    };

    template <T V>
    struct Value
    {
        constexpr static T operator()(size_t)
        {
            return V;
        }
    };

    // Helper functions
    template <size_t I, size_t... Idx>
    constexpr static TVector UnitImpl(std::index_sequence<Idx...>)
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
    constexpr static T BinaryOperation(Fn1 fn1, Fn2 fn2, I first, I second, std::index_sequence<Idx...>)
    {
        return fn1(fn2(first[Idx], second[Idx])...);
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

    inline static const TVector ZeroVector = TVector();

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
        return v.operator*(scale);
    }

    constexpr TVector operator/(const TVector& rhs) const
    {
        return BinaryOperation(std::divides<T>(), Data.begin(), rhs.Data.begin(), Indices);
    }

    constexpr T operator|(const TVector& rhs) const
    {
        return BinaryOperation(Summation(), std::multiplies<T>(), Data.begin(), rhs.Data.begin(), Indices);
    }

    constexpr TVector operator/(T scale) const
    {
        return this->operator*((T)1 / scale);
    }

    constexpr TVector& operator/=(T scale) 
    {
        return *this = *this / scale;
    }

    constexpr TVector& operator/=(const TVector& rhs) 
    {
        return *this = *this / rhs;
    }

    constexpr T operator[](size_t idx) const
    {
        return Data[idx];
    }

    constexpr T& operator[](size_t idx) 
    {
        return Data[idx];
    }

    // Static Methods(binary)
    constexpr static T Distance(const TVector& x, const TVector& y)
    {
        return EuclideanDistance(x.Data.begin(), y.Data.begin());
    }

    constexpr static TVector Max(const TVector& x, const TVector& y)
    {
        return BinaryOperation(MaxValue(), x.Data.begin(), y.Data.begin(), Indices);
    }

    constexpr static TVector Min(const TVector& x, const TVector& y)
    {
        return BinaryOperation(MinValue(), x.Data.begin(), y.Data.begin(), Indices);
    }

    constexpr static T DotProduct(const TVector& x, const TVector& y)
    {
        return x | y;
    }

    // Static Methods(unary)
    constexpr static TVector Abs(const TVector& v)
    {
        return UnaryOperation(AbsoluteValue(), v.Data.begin(), Indices);
    }

    constexpr static TVector Normalize(const TVector& v)
    {
        return BinaryOperation(std::multiplies<T>(), v.Data.begin(), (T)1 / Length(v), Indices);
    }

    constexpr static TVector SafeNormalize(const TVector& v, T Tolerance = SmallNumber)
    {
        const auto length = LengthSquare(v);
        return length > SmallNumber 
            ? BinaryOperation(std::multiplies<T>(), v.Data.begin(), (T)1 / std::sqrt(length), Indices)
            : TVector();
    }

    // template <typename Fn>
    // constexpr static TVector UserDefinedTransform(const TVector& v, Fn fn)
    // {
    //     return UnaryOperation(fn, v.Data().begin(), Indices);
    // }

    constexpr static TVector SignVector(const TVector& v)
    {
        return UnaryOperation(Sign(), v.Data.begin(), Indices);
    }

    constexpr static T Length(const TVector& v)
    {
        return Distance(v, TVector());
    }

    constexpr static T LengthSquare(const TVector& v)
    {
        return LDistance(Square(), Summation(), v.Data.begin(), ZeroVector.Data.begin(), Indices);
    }

    constexpr static Size(const TVector& v)
    {
        return Length(v);
    }

    constexpr static SizeSquare(const TVector& v)
    {
        return LengthSquare(v);
    }

    template <size_t I>
    constexpr static TVector Unit()
    {
        return UnitImpl<I>(Indices);
    }

    // Constructors
    constexpr TVector() = default;

    template <std::same_as<T>... Us>
    constexpr TVector(Us... us) : Data{ us... } { }

    // template <typename... Ts>
    // constexpr TVector(Ts... ts) : Data{ (T)ts... } { }

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