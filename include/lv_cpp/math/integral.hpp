#ifndef __INTEGRAL_HPP__
#define __INTEGRAL_HPP__

#include <exception>
#include <vector>
#include <utility>

namespace leviathan::exceptions
{
class diff_stencil_lower_error : std::exception
{
    const char* what() const noexcept
    {
        static const char* msg = "n should between 0 and 4";
        return msg;
    }
};

} // namespace exception

namespace leviathan::numeric
{
template<typename ValueType, typename FunctionType> 
ValueType diff_stencil_lower(ValueType x, FunctionType&& f,
    int n = 1, ValueType h = 0.001)
{
    if(n == 0)
        return f(x);
    else if(n == 1) // first order derivative
    {
		auto p =
			-f(x + 2.0 * h) + 8.0 * f(x + h) - 8.0 * f(x - h) + f(x - 2.0 * h);
		return p / (12.0 * h);
	}
    else if(n == 2) // second order
    {
		auto p = -f(x + 2.0 * h) + 16.0 * f(x + h) - 30.0 * f(x) +
				 16.0 * f(x - h) - f(x - 2.0 * h);
		return p / (12.0 * h * h);
	}
    else if(n == 3) // 3rd order
    {
		auto p =
			f(x + 2.0 * h) - 2.0 * f(x + h) + 2.0 * f(x - h) - f(x - 2.0 * h);
		return p / (2.0 * h * h * h);
	}
    else if(n==4) // 4-th order
    {
		auto p = f(x + 2.0 * h) - 4.0 * f(x + h) + 6.0 * f(x) - 4.0 * f(x - h) +
				 f(x - 2.0 * h);
		return p / (h * h * h * h);
	}
    else
    {
        throw exceptions::diff_stencil_lower_error{};
    }
}

template<typename ValueType, typename FunctionType> 
ValueType diff_stencil_higher(ValueType x, FunctionType&& f, int n = 1, ValueType h = 0.01)
{
	if (n == 0)
		return f(x);
    else if(n == 1) // first order derivative
    {
		auto p =
			-f(x + 2.0 * h) + 8.0 * f(x + h) - 8.0 * f(x - h) + f(x - 2.0 * h);
		return p / (12.0 * h);
	}
    else
    {
		auto f1 = diff_stencil_higher(x + 2.0 * h, f, n - 1);
		auto f2 = diff_stencil_higher(x + h, f, n - 1);
		auto f3 = diff_stencil_higher(x - h, f, n - 1);
		auto f4 = diff_stencil_higher(x - 2.0 * h, f, n - 1);

		auto p = -f1 + 8.0 * f2 - 8.0 * f3 + f4;
		return p / (12.0 * h);
	}
}

template<typename ValueType, typename FunctionType> 
ValueType diff_stencil(ValueType x, FunctionType&& f,
    int n = 1, ValueType h_lower = 0.001, ValueType h_higher = 0.01)
{
    if(n < 3)
        return diff_stencil_lower(x, f, n, h_lower);
    else
        return diff_stencil_higher(x, f, n, h_higher);    
}


template<typename ValueType>
auto weights_abscissae(int n = 5) -> std::vector<std::pair<ValueType, ValueType>>
{

    switch(n)
    {
        case 2: return {
                {1.0000000000000000, -0.5773502691896257},
                {1.0000000000000000, 0.5773502691896257} };

        case 3: return {
            {0.8888888888888888, 0.0000000000000000},
            {0.5555555555555556, -0.7745966692414834},
            {0.5555555555555556, 0.7745966692414834} };

        case 4: return {
            {0.6521451548625461, -0.3399810435848563},
            {0.6521451548625461, 0.3399810435848563},
            {0.3478548451374538, -0.8611363115940526},
            {0.3478548451374538, 0.8611363115940526} };

        case 5: return {
            {0.5688888888888889, 0.0000000000000000},
            {0.4786286704993665, -0.5384693101056831},
            {0.4786286704993665, 0.5384693101056831},
            {0.2369268850561891, -0.9061798459386640},
            {0.2369268850561891, 0.9061798459386640} };

        case 6: return {
            {0.3607615730481386, 0.6612093864662645},
            {0.3607615730481386, -0.6612093864662645},
            {0.4679139345726910, -0.2386191860831969},
            {0.4679139345726910, 0.2386191860831969},
            {0.1713244923791704, -0.9324695142031521},
            {0.1713244923791704, 0.9324695142031521} };

        case 7: return {
            {0.4179591836734694, 0.0000000000000000},
            {0.3818300505051189, 0.4058451513773972},
            {0.3818300505051189, -0.4058451513773972},
            {0.2797053914892766, -0.7415311855993945},
            {0.2797053914892766, 0.7415311855993945},
            {0.1294849661688697, -0.9491079123427585},
            {0.1294849661688697, 0.9491079123427585} };

        case 8: return {
            {0.3626837833783620, -0.1834346424956498},
            {0.3626837833783620, 0.1834346424956498},
            {0.3137066458778873, -0.5255324099163290},
            {0.3137066458778873, 0.5255324099163290},
            {0.2223810344533745, -0.7966664774136267},
            {0.2223810344533745, 0.7966664774136267},
            {0.1012285362903763, -0.9602898564975363},
            {0.1012285362903763, 0.9602898564975363} };

        case 9: return {
            {0.3302393550012598, 0.0000000000000000},
            {0.1806481606948574, -0.8360311073266358},
            {0.1806481606948574, 0.8360311073266358},
            {0.0812743883615744, -0.9681602395076261},
            {0.0812743883615744, 0.9681602395076261},
            {0.3123470770400029, -0.3242534234038089},
            {0.3123470770400029, 0.3242534234038089},
            {0.2606106964029354, -0.6133714327005904},
            {0.2606106964029354, 0.6133714327005904} };
        
        default: return {
            {0.2955242247147529, -0.1488743389816312},
            {0.2955242247147529, 0.1488743389816312},
            {0.2692667193099963, -0.4333953941292472},
            {0.2692667193099963, 0.4333953941292472},
            {0.2190863625159820, -0.6794095682990244},
            {0.2190863625159820, 0.6794095682990244},
            {0.1494513491505806, -0.8650633666889845},
            {0.1494513491505806, 0.8650633666889845},
            {0.0666713443086881, -0.9739065285171717},
            {0.0666713443086881, 0.9739065285171717} };
        }

        return {}; // to prevent warning
}

// Integrate function f from x1 (lower bound) to x2 (upper bound)
// using Gaussain Quadrature with n weights.
template<typename ValueType, typename FunctionType>
ValueType gaussian_quadrature(ValueType x1, ValueType x2,
    FunctionType&& f, int n = 7)
{
    auto wa = weights_abscissae<ValueType>(n);

    auto a = (x2 - x1) / 2.0;
    auto b = (x2 + x1) / 2.0;

    ValueType I{}; // initialize to zero

    for(auto&& [w, t]: wa)
    {
        I += w * f(a * t + b);
    }

    I *= a; return I;
}


} // namespace leviathan::numeric



#endif