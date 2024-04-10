#pragma once

#include "Vector.hpp"

namespace Leviathan::Math
{

template <typename T> using TVector2D = TVector<T, 2>;

struct Transform2D
{
    template <typename T>
    static TVector2D<T> RotateVector(const TVector2D<T>& v, std::type_identity_t<T> angle) 
    {
        // RotateMatrix:
        // [
        //   cosθ -sinθ
        //   sinθ cosθ
        // ]
        PrintTypeCategory(angle);

        const auto theta = angle / (T)180 * std::numbers::pi_v<T>;
        const auto cosine = std::cos(theta);
        const auto sine = std::sin(theta);
        const auto x = v.Data[0];
        const auto y = v.Data[1];
        return { cosine * x - sine * y, sine * x + cosine * y };
    }
};


} // namespace Leviathan::Math

