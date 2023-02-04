#pragma once

#include "../number/math.hpp"
#include <functional>

namespace lua
{
    
    template <auto Function, typename Return, typename Lhs = Return, typename Rhs = Return>
    struct BinaryOperation
    {
        Return operator()(Lhs lhs, Rhs rhs) const
        {
            return Function(lhs, rhs);
        }
    };

    inline constexpr auto plus = std::plus<>();
    inline constexpr auto minus = std::minus<>();
    inline constexpr auto multiplies = std::multiplies<>();
    inline constexpr auto divides = std::divides<>();
    inline constexpr auto bit_and = std::bit_and<>();
    inline constexpr auto bit_or = std::bit_or<>();
    inline constexpr auto bit_xor = std::bit_xor<>();

    #define GenerateBinaryOperation(name, return_type, functor) \
        inline constexpr auto lua_##name = BinaryOperation<functor, return_type>()

    // integer
    GenerateBinaryOperation(iadd, int64, plus);
    GenerateBinaryOperation(isub, int64, minus);
    GenerateBinaryOperation(imul, int64, multiplies);
    GenerateBinaryOperation(imod, int64, imod);

    // number
    GenerateBinaryOperation(fadd, float64, plus);
    GenerateBinaryOperation(fsub, float64, minus);
    GenerateBinaryOperation(fmul, float64, multiplies);
    GenerateBinaryOperation(fmod, float64, fmod);

    GenerateBinaryOperation(pow, float64, ::powl);
    GenerateBinaryOperation(div, float64, divides);
    GenerateBinaryOperation(iidiv, float64, ifloor_div);
    GenerateBinaryOperation(fidiv, float64, ffloor_div);

    GenerateBinaryOperation(band, int64, bit_and);
    GenerateBinaryOperation(bor, int64, bit_or);
    GenerateBinaryOperation(bxor, int64, bit_xor);

    GenerateBinaryOperation(shl, int64, shift_left);
    GenerateBinaryOperation(shr, int64, shift_right);

    #undef GenerateBinaryOperation

    template <auto Function, typename Return, typename Lhs = Return, typename Rhs = Return>
    struct UnaryOperation
    {
        Return operator()(Lhs lhs, Rhs rhs) const
        {
            return Function(lhs);
        }
    };

    inline constexpr auto negate = std::negate<>();
    inline constexpr auto bit_not = std::bit_not<>();

    #define GenerateUnaryOperation(name, return_type, functor) \
        inline constexpr auto lua_##name = UnaryOperation<functor, return_type>()

    GenerateUnaryOperation(iunm, int64, negate); 
    GenerateUnaryOperation(funm, float64, negate); 
    GenerateUnaryOperation(bnot, int64, bit_not); 

    #undef GenerateUnaryOperation

    struct Operator
    {
        std::function<int64(int64, int64)> integer_func;
        std::function<float64(float64, float64)> float_func;
    
        // int64 (*integer_func_ptr)(int64, int64); 
        // float64 (*integer_float_ptr)(float64, float64);
    };

    inline const Operator operators[] = {
        { lua_iadd, lua_fadd },
        { lua_isub, lua_fsub },
        { lua_imul, lua_fmul },
        { lua_imod, lua_fmod },
        { { }, lua_pow },
        { { }, lua_div },
        { lua_iidiv, lua_fidiv },
        { lua_band, { } },
        { lua_bor, { } },
        { lua_bxor, { } },
        { lua_shl, { } },
        { lua_shr, { } },
        { lua_iunm, lua_funm },
        { lua_bnot, { } },
    };



} // namespace lua

