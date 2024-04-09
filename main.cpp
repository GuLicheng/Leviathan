#include <iostream>

#include <Engine/Core/Math/Vector.hpp>

using namespace Leviathan::Math;

using FVector = VectorBase<float, 2>;

void func(FVector fv)
{
    fv.show();
    std::endl(std::cout);
}

using FVector3 = VectorBase<double, 4>;

void func(FVector3 fv)
{
    fv.show();
    std::endl(std::cout);
}

int main()
{

    FVector v1(0.0f, 0.0f), v2(1.0f, 1.0f);

    func(FVector::RotateVector(v1, 90));

    func(FVector::Basis<0>());
    func(FVector::Basis<1>());

    func(FVector3::Basis<0>());
    func(FVector3::Basis<1>());
    func(FVector3::Basis<2>());

    return 0;
}