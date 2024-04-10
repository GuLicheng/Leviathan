#include <iostream>

#include <Engine/Core/Vector2D.hpp>

using namespace Leviathan::Math;

using FVector = TVector2D<float>;


void func(auto fv)
{
    fv.show();
    std::endl(std::cout);
}

template<typename T>
void out(const T& value) 
{ 
    PrintTypeInfo(T);
    std::cout << value; 
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

    // out(FVector::Normalize(v2));
    std::cout << std::format("{:.3f}\n", FVector::Normalize(v2)) << '\n';


    return 0;
}