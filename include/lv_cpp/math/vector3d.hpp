#ifndef _TPF_VECTOR_HPP
#define _TPF_VECTOR_HPP

#define _USE_MATH_DEFINES
// #include <tpf_output.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <array>
	
namespace tpf
{

    template<typename ValueType, typename FunctionType> 
    ValueType diff_stencil_lower(ValueType x, FunctionType&& f,
        int n = 1, ValueType h = 0.001)
    {
        if(n==0)
            return f(x);
        else if(n == 1) // first order derivative
        {
            auto p = -f(x+2.0*h)+8.0*f(x+h)-8.0*f(x-h)+f(x-2.0*h);
            return p / (12.0*h);
        }
        else if(n == 2) // second order
        {
            auto p = -f(x+2.0*h)+16.0*f(x+h)-30.0*f(x)+16.0*f(x-h)-f(x-2.0*h);
            return p / (12.0*h*h);
        }
        else if(n == 3) // 3rd order
        {
            auto p = f(x+2.0*h)-2.0*f(x+h)+2.0*f(x-h)-f(x-2.0*h);
            return p /(2.0*h*h*h);
        }
        else if(n==4) // 4-th order
        {
            auto p = f(x+2.0*h)-4.0*f(x+h)+6.0*f(x)-4.0*f(x-h)+f(x-2.0*h);
            return p/(h*h*h*h);
        }
        else
        {
            // throw Tpf_DebugException("Derivative higher than 4th order is NOT supported");
            throw 0;
        }
    }

    template<typename ValueType, typename FunctionType> 
    ValueType diff_stencil_higher(ValueType x, FunctionType&& f,
        int n = 1, ValueType h = 0.01)
    {
        if(n==0)
            return f(x);
        else if(n == 1) // first order derivative
        {
            auto p = -f(x+2.0*h)+8.0*f(x+h)-8.0*f(x-h)+f(x-2.0*h);
            return p / (12.0*h);
        }
        else
        {
            auto f1 = diff_stencil_higher(x+2.0*h, f, n-1);
            auto f2 = diff_stencil_higher(x+h, f, n-1);
            auto f3 = diff_stencil_higher(x-h, f, n-1);
            auto f4 = diff_stencil_higher(x-2.0*h, f, n-1);

            auto p = -f1+8.0*f2-8.0*f3+f4;
            return p / (12.0*h);
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

    template<typename ElementType = double>
    class vector
    {
        public:
            using normal_t = vector<ElementType>;
            using unit_t = vector<ElementType>;

        protected:
            ElementType m_x, m_y, m_z;

        public:
            
            // C++20 introduced constexpr for constructor
            // it enables us to use vector as
            // a non-type template argument
            constexpr vector(ElementType x=ElementType{},
                ElementType y=ElementType{}, ElementType z=ElementType{}) noexcept:
                m_x{x}, m_y{y}, m_z{z} { }

            // default copy constructor
            vector(const vector&) = default;

            // default copy assignment
            vector& operator=(const vector&) = default;

        inline ElementType norm_square() const noexcept
        {
            return this->m_x * this->m_x + 
                this->m_y * this->m_y + 
                this->m_z * this->m_z;
        }

        inline ElementType norm() const noexcept
        {
            return std::sqrt(this->m_x * this->m_x + 
                this->m_y * this->m_y + 
                this->m_z * this->m_z);
        }

        // unit vector
        vector unit() const
        {
            auto mag = this->norm();
            return {this->m_x / mag, this->m_y / mag, this->m_z/mag};
        }

        // n should be unit vector
        vector parallel(const unit_t& n) const noexcept
        {
            // *this is vertex V
            // (*this * n) represents magnitude of P, the parallel vector
            // (*this * n) * n is P, the parallel component
            return (*this * n) * n;
        }

        // n should be unit vector
        vector orthogonal(const unit_t& n) const noexcept
        {
            return *this - this->parallel(n);
        }

        // split V into P and O, parallel and orthogonal components
        // respectively
        auto split(const unit_t& n) const noexcept
        {
            auto P = this->parallel(n);
            auto O = *this - P; 
            return std::pair<vector, vector>{P, O};
        }

        // projection V into P and O, parallel and orthogonal components
        // respectively
        auto projection(const unit_t& n) const noexcept
        {
            auto P = this->parallel(n);
            auto O = *this - P; 
            return std::pair<ElementType, vector>{P.norm_square(), O};
        }
     

/*
    	
    044 - OpenGL Graphics Tutorial 1 - Scaling factors and Translation factors
    https://www.youtube.com/watch?v=SctufBhw0Sc&list=PL1_C6uWTeBDF7kjfRMCmHIq1FncthhBpQ&index=44
    
    047 - OpenGL Graphics Tutorial 4 - Homogeneous Coordinates, Normalized Device Coordinates
	https://www.youtube.com/watch?v=bCCMb7uggwc&list=PL1_C6uWTeBDF7kjfRMCmHIq1FncthhBpQ&index=47

*/
        vector scaling(ElementType Sx,
            ElementType Sy, ElementType Sz) const noexcept
        {
            return {this->m_x * Sx, this->m_y * Sy, this->m_z * Sz };
        }

        vector translate(ElementType Tx,
            ElementType Ty, ElementType Tz) const noexcept
        {
            return {this->m_x + Tx, this->m_y + Ty, this->m_z + Tz };
        }

        vector scaling_translate(ElementType Sx,
            ElementType Sy, ElementType Sz, ElementType Tx,
            ElementType Ty, ElementType Tz) const noexcept
        {
            return {this->m_x * Sx + Tx, this->m_y * Sy + Ty, this->m_z * Sz + Tz };
        }

        // C Operator Precedence
	    // https://en.cppreference.com/w/c/language/operator_precedence

        // operator% is for Cross Product
        friend vector operator%(const vector& A, const vector& B) noexcept
            {
                return { A.m_y * B.m_z - A.m_z * B.m_y , // i
                         A.m_z * B.m_x - A.m_x * B.m_z , // j
                         A.m_x * B.m_y - A.m_y * B.m_x   /* k */ };
            }

        // operator* is for Dot Product
         friend ElementType operator*(const vector& A, const vector& B) noexcept
            {
                return A.m_x * B.m_x + A.m_y * B.m_y + A.m_z * B.m_z;
            }

        // operator+ is for vector addition
        // vector addition is vector translation
         friend vector operator+(const vector& A, const vector& B) noexcept
            {
                return { A.m_x + B.m_x, A.m_y + B.m_y, A.m_z + B.m_z};
                
            }

        // operator- is for vector subtraction
        // vector subtraction is "demotion to position" vector
         friend vector operator-(const vector& A, const vector& B) noexcept
            {
                return { A.m_x - B.m_x, A.m_y - B.m_y, A.m_z - B.m_z};
                
            }

        // scalar * vector multiplication - uniform scaling
        friend vector operator*(const ElementType& s, const vector& V) noexcept
            {
                return { s * V.m_x, s * V.m_y, s * V.m_z};
            }

        // vector * scalar multiplication - uniform scaling
        friend vector operator*(const vector& V, const ElementType& s) noexcept
            {
                return { s * V.m_x, s * V.m_y, s * V.m_z};
            }






        friend std::ostream& operator<<(std::ostream& os, const vector& v) noexcept
        {
            os<<"["<<v.m_x<<", "<<v.m_y<<", "<<v.m_z<<"]";
            return os;
        }
    };

    // compute normal vector of the plane
    // formed by vertices V1, V2, V3
    template<typename ElementType>
    auto normal_vector(const vector<ElementType>& V1,
        const vector<ElementType>& V2, const vector<ElementType>& V3)
    {
        auto D12 = V2 - V1;
        auto D23 = V3 - V1;

        return D12 % D23;    
    }

    template<typename ValueType>
    auto weights_abscissae(int n = 5)
    {
        using pair_t = std::pair<ValueType, ValueType>;
        using wa_t = std::vector<pair_t>;

        switch(n)
        {
            case 2: return wa_t{
                    {1.0000000000000000, -0.5773502691896257},
                    {1.0000000000000000, 0.5773502691896257} };

            case 3: return wa_t{
                {0.8888888888888888, 0.0000000000000000},
                {0.5555555555555556, -0.7745966692414834},
                {0.5555555555555556, 0.7745966692414834} };

            case 4: return wa_t{
                {0.6521451548625461, -0.3399810435848563},
                {0.6521451548625461, 0.3399810435848563},
                {0.3478548451374538, -0.8611363115940526},
                {0.3478548451374538, 0.8611363115940526} };

            case 5: return wa_t{
                {0.5688888888888889, 0.0000000000000000},
                {0.4786286704993665, -0.5384693101056831},
                {0.4786286704993665, 0.5384693101056831},
                {0.2369268850561891, -0.9061798459386640},
                {0.2369268850561891, 0.9061798459386640} };

            case 6: return wa_t{
                {0.3607615730481386, 0.6612093864662645},
                {0.3607615730481386, -0.6612093864662645},
                {0.4679139345726910, -0.2386191860831969},
                {0.4679139345726910, 0.2386191860831969},
                {0.1713244923791704, -0.9324695142031521},
                {0.1713244923791704, 0.9324695142031521} };

            case 7: return wa_t{
                {0.4179591836734694, 0.0000000000000000},
                {0.3818300505051189, 0.4058451513773972},
                {0.3818300505051189, -0.4058451513773972},
                {0.2797053914892766, -0.7415311855993945},
                {0.2797053914892766, 0.7415311855993945},
                {0.1294849661688697, -0.9491079123427585},
                {0.1294849661688697, 0.9491079123427585} };

            case 8: return wa_t{
                {0.3626837833783620, -0.1834346424956498},
                {0.3626837833783620, 0.1834346424956498},
                {0.3137066458778873, -0.5255324099163290},
                {0.3137066458778873, 0.5255324099163290},
                {0.2223810344533745, -0.7966664774136267},
                {0.2223810344533745, 0.7966664774136267},
                {0.1012285362903763, -0.9602898564975363},
                {0.1012285362903763, 0.9602898564975363} };

            case 9: return wa_t{
                {0.3302393550012598, 0.0000000000000000},
                {0.1806481606948574, -0.8360311073266358},
                {0.1806481606948574, 0.8360311073266358},
                {0.0812743883615744, -0.9681602395076261},
                {0.0812743883615744, 0.9681602395076261},
                {0.3123470770400029, -0.3242534234038089},
                {0.3123470770400029, 0.3242534234038089},
                {0.2606106964029354, -0.6133714327005904},
                {0.2606106964029354, 0.6133714327005904} };
            
            case 10: return wa_t{
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
            
            default:
                // Tpf_ThrowDebugException("Invalid argument n in weights_abscissae()");
                throw 0;
            }

            return wa_t{}; // to prevent warning
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

} // end of namespace tpf



#endif // end of file _TPF_VECTOR_HPP