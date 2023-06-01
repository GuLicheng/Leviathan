#ifndef __DOUBLE_INTEGRAL_HPP__
#define __DOUBLE_INTEGRAL_HPP__

#include <leviathan/math/integral.hpp>

namespace leviathan::numeric
{
// we should keep ValueType is float or double
// if you want, you can support long double
template <typename FunctionType, typename ValueType = double>
class function_xy
{
protected:
    static_assert(std::is_same_v<std::invoke_result_t<FunctionType, ValueType, ValueType>, ValueType>);
    static_assert(std::is_same_v<float, ValueType> || std::is_same_v<double, ValueType>);
    // may fixed someday



public:
    explicit function_xy(FunctionType f)
        : m_f{f} { }

    ValueType operator()(ValueType x, ValueType y) const
    {
        return this->m_f(x, y);
    }

    auto fix_x(ValueType x) const 
    {
        return [x, this](ValueType y)
        {
            return this->m_f(x, y);
        };
    }

    auto fix_y(ValueType y) const 
    {
        return [y, this](ValueType x)
        {
            return this->m_f(x, y);
        };
    }

    ValueType diff_x(ValueType x, ValueType y) const
    {
        auto y_fixed = this->fix_y(y);
        return diff_stencil<ValueType>(x, y_fixed);
    }

    ValueType diff_y(ValueType x, ValueType y) const
    {
        auto x_fixed = this->fix_x(x);
        return diff_stencil<ValueType>(y, x_fixed);
    }

    std::pair<ValueType, ValueType> gradient(ValueType x, ValueType y) const
    {
        return {this->diff_x(x, y), this->diff_y(x, y)};
    }

    template <typename Lower, typename Upper>
    auto integrate_x(Lower lower, Upper upper) const
    {        
        return [=](auto y)
        {
            auto y_fixed = this->fix_y(y);
            auto a = evalute<ValueType>(lower, y);
            auto b = evalute<ValueType>(upper, y);
            return gaussian_quadrature<ValueType>(a, b, y_fixed);
        }; // int_y 
    }

    template <typename Lower, typename Upper>
    auto integrate_y(Lower lower, Upper upper) const
    {
        return [=](auto x)
        {
            auto x_fixed = this->fix_x(x);
            auto a = evalute<ValueType>(lower, x);
            auto b = evalute<ValueType>(upper, x);
            return gaussian_quadrature<ValueType>(a, b, x_fixed);
        }; // int_x    
    }

    template <typename X_lower, typename X_upper>
    ValueType integrate_xy(X_lower&& x_lower, X_upper&& x_upper, ValueType y_lower, ValueType y_upper)
    {
        // integrate x and then y
        auto int_y = this->integrate_x(x_lower, x_upper);  // dx
        return gaussian_quadrature<ValueType>(y_lower, y_upper, int_y);  // dy
    }

    template <typename Y_lower, typename Y_upper>
    ValueType integrate_yx(Y_lower&& y_lower, Y_upper&& y_upper, ValueType x_lower, ValueType x_upper)
    {
        // integrate y and then x
        auto int_x = this->integrate_y(y_lower, y_upper);  // dy
        return gaussian_quadrature<ValueType>(x_lower, x_upper, int_x);  // dx
    }

    auto fix_x1(ValueType x1) const
    {
        return [=](ValueType x2)
        {
            return this->m_f(x1, x2);
        };
    }

    // we fix x2-component
    auto fix_x2(ValueType x2) const
    {
        return [=](ValueType x1)
        {
            return this->m_f(x1, x2);
        };
    }

    template<typename TypeX21, typename TypeX22>
    ValueType integrate(ValueType x11, ValueType x12, TypeX21&& x21, TypeX22&& x22)
    {
        // inner integral of double integrals
        auto inner = [=](auto&& x1)
        {
            // evaluate lower of bound x21 using x1 as its argument
            // In double integral, x21 and x22 can be functions of x1.
            auto a = evalute<ValueType>(x21, x1);
            auto b = evalute<ValueType>(x22, x1);

            return gaussian_quadrature(a, b, this->fix_x1(x1));
        };

        return gaussian_quadrature(x11, x12, inner);
    }

private:
    FunctionType m_f;  
};



}


#endif