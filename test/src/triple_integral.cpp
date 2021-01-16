#include <lv_cpp/math/triple_integral.hpp>
#include <iostream>
#include <cmath>
#define M_PI 3.141592653589793

using namespace leviathan::numeric;

void test1()
{
    auto f =[](auto&& x1, auto&& x2, auto&& x3)
    {
        return 1.0;
    };

    auto x31 = [](auto&& x1, auto&& x2)
    {
        // x^2 + 3 * y^2
        return x1 * x1 + 3.0 * x2 * x2;
    };

    auto x32 = [](auto&& x1, auto&& x2)
    {
        // 8 - x^2 - y^2
        return 8.0 - x1 * x1 - x2 * x2;
    };

    auto x21 = [](auto&& x1)
    {
        return - std::sqrt( (4.0 - x1 * x1) / 2.0);
    };

    auto x22 = [](auto&& x1)
    {
        return std::sqrt( (4.0 - x1 * x1) / 2.0);
    };

    function_xyz fxyz{ f };

    auto rlt = fxyz.integrate(-2.0, 2.0, x21, x22, x31, x32);
    auto ext = 8.0 * M_PI * std::sqrt(2.0);

    std::cout << "Our approximation: " << rlt << std::endl;
    std::cout << "Exact computation: " << ext << std::endl;

    std::cout << "Error of difference: " << std::fabs(rlt - ext) << std::endl;
}

void test2()
{
    auto f =[](auto&& x1, auto&& x2, auto&& x3)
    {
        return 1.0;
    };

    auto x31 = 0.0;

    auto x32 = [](auto&& x1, auto&& x2)
    {
        // y - x
        return x2 - x1;
    };

    auto x21 = [](auto&& x1)
    {
        return x1;
    };

    auto x22 = 1.0;

    function_xyz fxyz{ f };

    auto rlt = fxyz.integrate(0.0, 1.0, x21, x22, x31, x32);
    auto ext = 1.0 / 6.0;

    std::cout << "Our approximation: " << rlt << std::endl;
    std::cout << "Exact computation: " << ext << std::endl;

    std::cout << "Error of difference: " << std::fabs(rlt - ext) << std::endl;
}

void test3()
{
    auto f =[](auto&& x1, auto&& x2, auto&& x3)
    {
        return x1 * x2 * x3;
    };

    auto x31 = 0.0;
    auto x32 = 2.0;

    auto x21 = 0.0;
    auto x22 = 2.0;

    function_xyz fxyz{ f };

    auto rlt = fxyz.integrate(0.0, 2.0, x21, x22, x31, x32);
    auto ext = 8.0;

    std::cout << "Our approximation: " << rlt << std::endl;
    std::cout << "Exact computation: " << ext << std::endl;

    std::cout << "Error of difference: " << std::fabs(rlt - ext) << std::endl;
}

int main()
{
    test1();
    test2();
    test3();
}