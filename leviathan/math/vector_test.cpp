#include "vector.hpp"

#include <catch2/catch_all.hpp>

template class leviathan::math::vector<float, 3>;
template class leviathan::math::vector<double, 3>;

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

    constexpr FVector v1(1, 1, 1), v2(1, 2, 3), v3(0, 0, 0), v4(-1, 2, -3);

    REQUIRE(CheckEqual(v1 + v2, FVector(2, 3, 4)));

    REQUIRE(CheckEqual(v2 - v1, FVector(0, 1, 2)));

    REQUIRE(CheckEqual(v1 * v3, v3));
    REQUIRE(CheckEqual(v1 * 2, FVector(2, 2, 2)));
    REQUIRE(CheckEqual(2 * v1, FVector(2, 2, 2)));
    
    REQUIRE(CheckEqual(v2 / FVector(2, 2, 2), FVector(0.5, 1, 1.5)));
    REQUIRE(CheckEqual(v2 / 2, FVector(0.5, 1, 1.5)));

    REQUIRE(FloatingCmpEqual(v1 | v3, 0));
    REQUIRE(FloatingCmpEqual(v1 | v2, 6));

    REQUIRE(CheckEqual(-v2, FVector(-1, -2, -3)));

    REQUIRE(CheckEqual(FVector::abs(v4), FVector(1, 2, 3)));
    REQUIRE(CheckEqual(FVector::sign_vector(v4), FVector(-1, 1, -1)));

    REQUIRE(FloatingCmpEqual(FVector::size_square(v1), 3));
    REQUIRE(FloatingCmpEqual(FVector::size(v1), std::sqrt(3)));

    REQUIRE(CheckEqual(FVector::max(v2, v4), FVector(1, 2, 3)));
    REQUIRE(CheckEqual(FVector::min(v2, v4), FVector(-1, 2, -3)));
    
}

TEST_CASE("3D vector floating test binary operation")
{
    Vector3DTesting<float>();
    Vector3DTesting<double>();
}


