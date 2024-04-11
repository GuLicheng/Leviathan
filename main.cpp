#include <iostream>

#include <Engine/Core/Transform.hpp>

using namespace Leviathan::Math;

using FVector = TVector2D<float>;

template class TVector<float, 2>;

void func(auto fv)
{
    std::cout << std::format("{}\n", fv);
}


int main()
{

    FVector v1(0.0f, 0.0f), v2(1.0f, 1.0f), v3(1.0f, 2.f), v4(-1.f, -1.f);

    func(Transform2D::RotateVector(v2, 90));

    using FVector3 = TVector<double, 4>;

    func(FVector3::Unit<0>());
    func(FVector3::Unit<1>());
    func(FVector3::Unit<2>());

    // out(FVector::Normalize(v2));
    std::cout << std::format("{:.3f}\n", FVector::Normalize(v2));
    std::cout << std::format("{:.3f}\n", FVector::Min(v1, v2));
    std::cout << std::format("{:.3f}\n", FVector::Max(v1, v2));
    std::cout << std::format("{:.3f}\n", FVector::DotProduct(v3, v2));
    std::cout << std::format("{:.3f}\n", FVector::Abs(v4));

    return 0;
}