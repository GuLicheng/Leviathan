#ifndef __DOUBLE_INTEGRAL_HPP__
#define __DOUBLE_INTEGRAL_HPP__

#include <lv_cpp/math/integral.hpp>

namespace leviathan::numeric
{
// we should keep ValueType is float or double
// if you want, you can support long double
template <typename FunctionType, typename ValueType = double>
class function_xy
{
    static_assert(std::is_same_v<std::invoke_result_t<FunctionType, ValueType, ValueType>, ValueType>);
    static_assert(std::is_same_v<float, ValueType> || std::is_same_v<double, ValueType>);
    // may fixed someday

    // explicit make ReturnType to convert ValueType
    // Args can only be floating so it's not necessary pass by reference
    template <typename ReturnType, typename CallableOrNot, typename... Args>
    ReturnType evalute(CallableOrNot call, Args... args) const 
    {
        if constexpr (std::is_invocable_v<CallableOrNot, Args...>)
        {
            return call(args...);
        }
        else
        {   
            // cannot be callable
            return call;
        }
    }

public:
    explicit function_xy(FunctionType f, ValueType dummy_value = ValueType{})
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
            auto a = this->evalute<ValueType>(lower, y);
            auto b = this->evalute<ValueType>(upper, y);
            return gaussian_quadrature<ValueType>(a, b, y_fixed);
        }; // int_y 
    }

    template <typename Lower, typename Upper>
    auto integrate_y(Lower lower, Upper upper) const
    {
        return [=](auto x)
        {
            auto x_fixed = this->fix_x(x);
            auto a = this->evalute<ValueType>(lower, x);
            auto b = this->evalute<ValueType>(upper, x);
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

private:
    FunctionType m_f;  
};



}


#endif