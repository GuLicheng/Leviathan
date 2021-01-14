#ifndef __TRIPLE_INTEGRAL_HPP__
#define __TRIPLE_INTEGRAL_HPP__


#include <lv_cpp/math/double_integral.hpp>
#include <tuple>

namespace leviathan::numeric
{
template <typename FunctionType, typename ValueType = double>
class function_xyz
{
private:
    FunctionType m_f; // our function with two independent variables, x and y

public:
    function_xyz(FunctionType f): m_f{f} { }

    ValueType operator()(ValueType x, ValueType y, ValueType z) const
    {
        return this->m_f(x, y, z);
    }

    // we fix x-component to x
    auto fix_x(ValueType x) const
    {
        return [=](ValueType y, ValueType z)
        {
            return this->m_f(x, y, z);
        };
    }

    // we fix y-component to y
    auto fix_y(ValueType y) const
    {
        return [=](ValueType x, ValueType z)
        {
            return this->m_f(x, y, z);
        };
    }

    // we fix z-component to z
    auto fix_z(ValueType z) const
    {
        return [=](ValueType x, ValueType y)
        {
            return this->m_f(x, y, z);
        };
    }

    ////////////////////////////////////////////////////////
    // we fix x1-component
    auto fix_x1(ValueType x1) const
    {
        return [=](ValueType x2, ValueType x3)
        {
            return this->m_f(x1, x2, x3);
        };
    }

    // we fix x2-component
    auto fix_x2(ValueType x2) const
    {
        return [=](ValueType x1, ValueType x3)
        {
            return this->m_f(x1, x2, x3);
        };
    }

    // we fix x3-component
    auto fix_x3(ValueType x3) const
    {
        return [=](ValueType x1, ValueType x2)
        {
            return this->m_f(x1, x2, x3);
        };
    }
    ///////////////////////////////////////////////////////

    auto fix_xy(ValueType x, ValueType y) const
    {
        return [=](ValueType z)
        {
            return this->m_f(x, y, z);
        };
    }

    auto fix_yz(ValueType y, ValueType z) const
    {
        return [=](ValueType x)
        {
            return this->m_f(x, y, z);
        };
    }

    auto fix_xz(ValueType x, ValueType z) const
    {
        return [=](ValueType y)
        {
            return this->m_f(x, y, z);
        };
    }

    // partial derivative WRT x at (x, y, z)
    ValueType diff_x(ValueType x, ValueType y, ValueType z) const
    {
        auto yz_fixed = this->fix_yz(y, z);
        return diff_stencil(x, yz_fixed);
    }

    // partial derivative WRT y at (x, y, z)
    ValueType diff_y(ValueType x, ValueType y, ValueType z) const
    {
        auto xz_fixed = this->fix_xz(x, z);
        return diff_stencil(y, xz_fixed);
    }

    // partial derivative WRT z at (x, y, z)
    ValueType diff_z(ValueType x, ValueType y, ValueType z) const
    {
        auto xy_fixed = this->fix_xy(x, y);
        return diff_stencil(z, xy_fixed);
    }

    std::tuple<ValueType, ValueType, ValueType> 
    gradient(ValueType x, ValueType y, ValueType z) const
    {
        return { this->diff_x(x, y, z), 
                 this->diff_y(x, y, z), 
                 this->diff_z(x, y, z) };
    }

    // we are integrating Triple Integral
    // x21, x22 can be functions of x1
    // x31, x32 can be functions of x1, x2
    template<typename TypeX21, typename TypeX22, typename TypeX31, typename TypeX32>
    ValueType integrate(ValueType x11, ValueType x12,
        TypeX21&& x21, TypeX22&& x22, TypeX31&& x31, TypeX32&& x32)
    {
        auto inner_double_integral = [&x21, &x22, &x31, &x32, this](auto&& x1)
        {
            auto a = evalute<ValueType>(x21, x1);
            auto b = evalute<ValueType>(x22, x1);

            auto y1 = [&x31, x1](auto&& x2)
            {
                return evalute<ValueType>(x31, x1, x2);
            };

            auto y2 = [&x32, x1](auto&& x2)
            {
                return evalute<ValueType>(x32, x1, x2);
            };

            // function_xy to evaluate double integral
            function_xy fxy{ this->fix_x1(x1) };

            return fxy.integrate(a, b, y1, y2);
        };

        // this forms powerful triple integral
        return gaussian_quadrature(x11, x12, inner_double_integral);
    }
}; // end of class function_xyz


}  //  namespace numeric



#endif