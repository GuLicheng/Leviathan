#pragma once

#include "Vector.hpp"

namespace Leviathan::Math
{
    template <typename T>
    struct Vector4D
    {
        T X;
        T Y;
        T Z;
        T W;

        Vector4D() = default;

        Vector4D(T x, T y, T z, T w) : X(x), Y(y), Z(z), W(w) { }

        Vector4D operator+(const Vector4D& rhs) 
        {
            return { X + rhs.X, Y + rhs.Y, Z + rhs.Z, W + rhs.W };
        }

        Vector4D& operator+=(const Vector4D& rhs)
        {
            return *this = *this + rhs;
        }

        static T Distance(const Vector4D& lhs, const Vector4D& rhs)
        {
            const auto X = lhs.X - rhs.X;
            const auto Y = lhs.Y - rhs.Y;
            const auto Z = lhs.Z - rhs.Z;
            const auto W = lhs.W - rhs.W;
            return std::sqrt(X * X + Y * Y + Z * Z + W * W);
        }
    };

}
