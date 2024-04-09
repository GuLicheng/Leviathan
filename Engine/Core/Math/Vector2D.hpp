#pragma once

#include "Vector.hpp"

namespace Leviathan::Math
{
    template <typename T>
    struct Vector2D : VectorBase<T, 2>
    {
        using Super = VectorBase<T, 2>;
        using Super::Super;
        using Super::operator=;

        constexpr static Vector2D Zero = { T(0), T(0) };
        
        constexpr static Vector2D One = { T(1), T(1) };

        constexpr static Vector2D Up = { T(0), T(1) };

        constexpr static Vector2D Down = { T(0), T(-1) };

        constexpr static Vector2D Left = { T(-1), T(0) };

        constexpr static Vector2D Right = { T(1), T(0) };

        constexpr T X() const
        { 
            return this->Data[0]; 
        }

        constexpr T Y() const
        {
            return this->Data[1];
        }

        static Vector2D RotateVector(const Vector2D& vector, T angle)
        {
            // RotateMatrix:
            // [
            //   cosθ -sinθ
            //   sinθ cosθ
            // ]
            const auto theta = angle / (T)180 * std::numbers::pi_v<T>;
            const auto cosine = std::cos(theta);
            const auto sine = std::sin(theta);
            const auto x = vector.Data[0];
            const auto y = vector.Data[1];
            return { cosine * x - sine * y, sine * x + cosine * y };
        }
    };


} // namespace Leviathan::Math
