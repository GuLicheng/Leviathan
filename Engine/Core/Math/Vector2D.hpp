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

        Vector2D(Super super) : Super(super) { }

        constexpr static Vector2D Zero = { T(0), T(0) };
        
        constexpr static Vector2D One = { T(1), T(1) };

        constexpr static Vector2D Up = { T(0), T(1) };

        constexpr static Vector2D Down = { T(0), T(-1) };

        constexpr static Vector2D Left = { T(-1), T(0) };

        constexpr static Vector2D Right = { T(1), T(0) };
    };
} // namespace Leviathan::Math
