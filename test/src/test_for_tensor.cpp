#include <iostream>
#include <lv_cpp/math/tensor.hpp>

int main()
{
    ::leviathan::math::tensor<float, 3> tensor;

    // for (int i = 0; i < 3; ++i)
    //     for (int j = 0; j < 3; ++j)
    //         for (int k = 0; k < 3; ++k)
    //             std::cout << tensor[i][j][k] << ' ';

    for (auto first : tensor)
        for (auto second : first)
            for (auto third : second)
                std::cout << third << ' ';

    std::cout << "hello world\n";
}