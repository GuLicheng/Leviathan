#include <iostream>

#include <Engine/Core/Vector2D.hpp>

using namespace Leviathan::Math;

using FVector = TVector2D<float>;


void func(auto fv)
{
    fv.show();
    std::endl(std::cout);
}

int main()
{

    FVector v1(0.0f, 0.0f), v2(1.0f, 1.0f);

    func(Transform2D::RotateVector(v2, 90));

    

    // func(FVector::Basis<0>());
    // func(FVector::Basis<1>());


    // using FVector3 = TVector<double, 4>;

    // func(FVector3::Basis<0>());
    // func(FVector3::Basis<1>());
    // func(FVector3::Basis<2>());

    // func(FVector::Normalize(v2));

    return 0;
}