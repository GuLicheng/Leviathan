#include <iostream>

#include <Engine/Core/Math/Vector2D.hpp>

using namespace Leviathan::Math;

// template class Vector<float, 2>;

using FVector = Vector2D<float>;

int main()
{

    FVector v1(0.0f, 0.0f), v2(1.0f, 1.0f);

    auto v3 = FVector::RotateVector(v2, 90);

    v3.show();

    // std::cout << v3.Data[0] << '\n';
    // std::cout << v3.Data[1] << '\n';
    // std::cout << v3.Data[2] << '\n';

    std::cout << FVector::Distance(v1, v2) << '\n';

    (FVector::One / 2).show();

    std::cout << FVector::Norm(v2) << '\n';

    return 0;
}