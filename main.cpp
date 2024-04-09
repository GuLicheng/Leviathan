#include <iostream>

#include <Engine/Core/Math/Vector.hpp>

using namespace Leviathan::Math;

using FVector = VectorBase<float, 2>;

void func(FVector fv)
{
    fv.show();
}

int main()
{

    FVector v1(0.0f, 0.0f), v2(1.0f, 1.0f);

    func(FVector::RotateVector(v1, 90));

    return 0;
}