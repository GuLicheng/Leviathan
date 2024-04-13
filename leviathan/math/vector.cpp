#include "vector.hpp"

#include <catch2/catch_all.hpp>

template <typename Vector>
constexpr bool CheckEqual(Vector x1, Vector x2, double Tolerance = 1e-5)
{
    for (size_t i = 0; i < Vector::dimension(); ++i)
        if (std::abs(x1[i] - x2[i]) > (typename Vector::value_type)Tolerance)
            return false;
    return true;
}

constexpr bool FloatingCmpEqual(float x1, float x2, float Tolerance = 1e-5)
{
    return std::abs(x1 - x2) < Tolerance;
}

template <typename Floating>
constexpr void Vector3DTesting()
{
    using FVector = leviathan::math::vector<Floating, 3>;

    constexpr FVector v1(1, 1, 1), v2(1, 2, 3), v3(0, 0, 0);

    static_assert(CheckEqual(v1 + v2, FVector(2, 3, 4)));

    static_assert(CheckEqual(v2 - v1, FVector(0, 1, 2)));

    static_assert(CheckEqual(v1 * v3, v3));
    static_assert(CheckEqual(v1 * 2, FVector(2, 2, 2)));
    static_assert(CheckEqual(2 * v1, FVector(2, 2, 2)));
    
    static_assert(CheckEqual(v2 / FVector(2, 2, 2), FVector(0.5, 1, 1.5)));
    static_assert(CheckEqual(v2 / 2, FVector(0.5, 1, 1.5)));

    static_assert(FloatingCmpEqual(v1 | v3, 0));
    static_assert(FloatingCmpEqual(v1 | v2, 6));

    static_assert(CheckEqual(-v2, FVector(-1, -2, -3)));
}

TEST_CASE("3D vector floating test binary operation")
{
    Vector3DTesting<float>();
    Vector3DTesting<double>();
}


