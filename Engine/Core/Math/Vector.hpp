#pragma once

#include <cstdint>
#include <cmath>
#include <concepts>
#include <type_traits>
#include <array>
#include <functional>

namespace Leviathan::Math
{

// T may only be float32, float64 or float16?
// N may be one of [2, 3, 4]
template <std::floating_point T, size_t N>    
struct VectorBase
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

    template <typename UnaryOp, typename I, size_t... Idx>
    constexpr static VectorBase UnaryOperation(UnaryOp fn, I first, std::index_sequence<Idx...>)
    {
        return { fn(first[Idx])... };
    }

    template <typename BinaryOp, typename I, size_t... Idx>
    constexpr static VectorBase BinaryOperation(BinaryOp fn, I first, I second, std::index_sequence<Idx...>)
    {
        return { fn(first[Idx], second[Idx])... };
    }

    template <typename BinaryOp, typename I, size_t... Idx>
    constexpr static VectorBase BinaryOperation(BinaryOp fn, I first, T scale, std::index_sequence<Idx...>)
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
    // Operators(vector and vector)
    constexpr VectorBase operator+(const VectorBase& rhs) const
    {
        return BinaryOperation(std::plus<T>(), Data.begin(), rhs.Data.begin(), Indices);
    }

    constexpr VectorBase operator-(const VectorBase& rhs) const
    {
        return BinaryOperation(std::minus<T>(), Data.begin(), rhs.Data.begin(), Indices);
    }

    constexpr VectorBase operator-() const
    { 
        return UnaryOperation(std::negate<T>(), Data.begin(), Indices);
    }

    constexpr VectorBase operator*(const VectorBase& rhs) const
    {
        return BinaryOperation(std::multiplies<T>(), Data.begin(), rhs.Data.begin(), Indices);
    }

    constexpr VectorBase& operator+=(const VectorBase& rhs)
    {
        return *this = *this + rhs;    
    }

    constexpr VectorBase& operator-=(const VectorBase& rhs)
    {
        return *this = *this - rhs;    
    }

    constexpr VectorBase& operator*=(const VectorBase& rhs)
    {
        return *this = *this * rhs;    
    }

    // Operators(vector and scale)
    constexpr VectorBase operator*(T scale) const
    {
        return BinaryOperation(std::multiplies<T>(), Data.begin(), scale, Indices);
    }

    friend constexpr VectorBase operator*(T scale, const VectorBase& v)
    {
        return v * scale;
    }

    constexpr VectorBase operator/(T scale) const
    {
        return BinaryOperation(std::divides<T>(), Data.begin(), scale, Indices);
    }

    constexpr VectorBase& operator/=(T scale) 
    {
        return *this = *this / scale;
    }

    // Static Methods
    constexpr static T Distance(const VectorBase& lhs, const VectorBase& rhs)
    {
        return EuclideanDistance(lhs.Data.begin(), rhs.Data.begin());
    }

    constexpr static VectorBase Normalize(const VectorBase& v)
    {
        return BinaryOperation(std::divides<T>(), v.Data.begin(), Length(v), Indices);
    }

    constexpr static T Length(const VectorBase& v)
    {
        return Distance(v, VectorBase());
    }

    // Constructors
    constexpr VectorBase() = default;

    template <std::same_as<T>... Us>
    constexpr VectorBase(Us... us) : Data{ us... } { }

    // Other Methods
    void show() const
    {
        const char* delimiter = "(";
        for (int i = 0; i < N; ++i)
        {
            std::cout << std::__exchange(delimiter, ", ") << Data[i];
        }
        std::cout << ')';
    }

    std::array<T, N> Data;   
};  

} // namespace Math


