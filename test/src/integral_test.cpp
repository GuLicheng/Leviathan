#include <lv_cpp/math/double_integral.hpp>
#include <vector>
#include <iostream>
#include <cmath>

using namespace leviathan::numeric;
constexpr double pi = 3.1415926535897932384626433833;

void test1()
{
    auto f = [](auto&& x)
    {
        return std::sin(x);
    };
    auto I = gaussian_quadrature(0.0, pi / 4, f);
    // integral sin(x) from 0 to pi/4 is (2 - √2) / 2
    auto C = (2.0 - std::sqrt(2.0)) / 2.0;
    constexpr auto double_max_precision = std::numeric_limits<double>::digits10;
    std::cout.precision(double_max_precision);
    std::cout << "I = " << I << std::endl;
    std::cout << "C = " << C << std::endl;
    std::cout << "The error is " << std::abs(I - C) << std::endl;
}

void test2()
{
    auto f_ex2 = [](auto x, auto y)
    {
        // xe^y + cos(xy)
        // df/dx = e^y - ysin(xy)
        // df/dy = xe^y -xsin(xy)
        return x * std::exp(y) + std::cos(x * y);
    };
    function_xy fxy{f_ex2};

    // the partial derivatives of f are everywhere continuous and at(2.0) are 
    // given by f_x(2,0) = 1, f_y(2, 0) = 2

    std::cout << "fxy(2, 0) = " << fxy(2.0, .0) << std::endl; // 3
    std::cout << "f_x(2, 0) = " << fxy.diff_x(2, 0) << std::endl;  // 1
    std::cout << "f_y(2, 0) = " << fxy.diff_y(2, 0) << std::endl;  // 2

    auto [x, y] = fxy.gradient(2.0, .0);  // (1, 2) -> i + 2j
    std::cout << "(" << x << "," << y << ")" << std::endl;


    // the derivative of f at(2, 0) in the direction of v is therefore
    // Duf|(2, 0) = Gradf|(2.0) · u, where u is v unitization 
    // assume v = (3, -4), u = (3/5, -4/5)
    // Gradf|(2.0) · u = (i + 2j)(3/5i - 4/5j) = 3/5 - 8/5 = -1
    auto Duf = x * 0.6 + y * -0.8;
    std::cout << "Duf(2, 0) = " << Duf << std::endl;
    std::cout << "Work Successfully\n";
}

void test3()
{
    //  Volume = (4-x-y)dxdy 
    // where x is from 0 to 2 and y is from 0 to 1
    auto f = [](auto x, auto y)
    {
        return 4.0 - x - y;
    };


    function_xy fxy{f};
    auto i_dx_dy = fxy.integrate_xy(.0, 2.0, 0, 1.0);
    auto i_dy_dx = fxy.integrate_yx(.0, 1.0, 0, 2.0);

    std::cout << "integrate dxdy = " << i_dx_dy << std::endl;
    std::cout << "integrate dydx = " << i_dy_dx << std::endl;



    // for circle C with radius = 1, the area of C is pi
    auto area = [](auto r, auto theta)
    {
        return r;  //  r x dr x dtheta where r is from 0 to radius and theta is from 0 to 2pi
    };
    constexpr double radius = 1;
    function_xy circle{area};
    // for circle C with radius 1, its area is pi
    auto res = circle.integrate_xy(.0, radius, .0, 2 * pi);
    std::cout << "The area of circle is : " << res << std::endl;


}

void test4()
{
    // calculate sin(x) / x dS
    // where y is from 0 to x 
    // and x is from 0 to 1

    // we define G = sin(x)/x dydx 
    // where y is from 0 to x and 
    // x is from 0 to 1
    auto f = [](auto x, auto y)
    {
        return std::sin(x) / x;
    };
    function_xy fxy{f};

    double y_lower = .0;
    auto y_upper = [](auto x) { return x; };

    auto dydx = fxy.integrate_yx(y_lower, y_upper, .0, 1.0);
    std::cout << "dydx = " << dydx << std::endl;
    // if we reverse the order of integration and
    // attempt to calculate G' = six(x)/x dxdy
    // where x is from y to 1 and 
    // y is from 0 to 1, we will run into a problem
    // because the inner intergration{six(x)/x dx}
    // cannot be expressed in


    auto x_lower = [](auto y) { return y; };
    auto x_upper = 1.0;

    auto dxdy = fxy.integrate_xy(x_lower, x_upper, .0, 1.0);
    std::cout << "dxdy = " << dxdy << std::endl;
}

void test5()
{
    // F = 4x+2 dydx where 
    // y is from x^2 to 2*x
    // and x is from 0 to 2

    auto f = [](auto x, auto y){ return 4 * x + 2;};
    function_xy fxy{f};

    auto y_lower = [](auto x) { return x*x; };
    auto y_upper = [](auto x) { return x*2; };
    auto int1 = fxy.integrate_yx(y_lower, y_upper, 0.0, 2.0);

    // reverse the order of dx and dy
    // x' is from y/2 to √y and y is from 0 to 4
    // the result should be 8
    auto x_lower = [](auto y){ return y/2.0;};
    auto x_upper = [](auto y) { return ::sqrt(y); };
    auto int2 = fxy.integrate_xy(x_lower, x_upper, .0, 4.0);

    std::cout << "int1 = " << int1 << std::endl;
    std::cout << "int2 = " << int2 << std::endl;

}

int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
}