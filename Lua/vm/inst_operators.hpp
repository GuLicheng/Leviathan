#pragma once

#include "../api/constants.hpp"

namespace lua
{

    template <typename VM, typename InstructionT>
    class OperatorInstructionsInterface
    {

        VM& vm() 
        {
            return static_cast<VM&>(*this);
        }

        void binary_arith_impl(InstructionT ins, LuaArithmeticOp op)
        {
            auto [a, b, c] = ins.ABC();
            a++;
            this->vm().get_RK(b);
            this->vm().get_RK(c);
            this->vm().arith(op);
            this->vm().replace(a);
        }

        void unary_arith_impl(InstructionT ins, LuaArithmeticOp op)
        {
            auto [a, b, _] = ins.ABC();
            a++;
            b++;
            this->vm().push_value(b);
            this->vm().arith(op);
            this->vm().replace(a);
        }


        void compare_impl(InstructionT ins, LuaComparisonOp op)
        {
            auto [a, b, c] = ins.ABC();
            this->vm().get_RK(b);
            this->vm().get_RK(c);
            if (this->vm().compare(-2, -1, op) != (a != 0))
                this->vm().add_PC(1);
            this->vm().pop(2);
        }

    public:

        using enum LuaArithmeticOp;
        using enum LuaComparisonOp;

        void add(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPADD); }
        void sub(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPSUB); }
        void mul(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPMUL); }
        void mod(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPMOD); }
        void pow(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPPOW); }
        void div(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPDIV); }
        void idiv(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPIDIV); }
        void band(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPBAND); }
        void bor(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPBOR); }
        void bxor(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPBXOR); }
        void shl(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPSHL); }
        void shr(InstructionT ins) { return this->binary_arith_impl(ins, LUA_OPSHR); }

        void unm(InstructionT ins) { return this->unary_arith_impl(ins, LUA_OPUNM); }
        void bnot(InstructionT ins) { return this->unary_arith_impl(ins, LUA_OPBNOT); }

        void concats(InstructionT ins)
        {
            auto [a, b, c] = ins.ABC();
            a++;
            b++;
            c++;
            auto n = c - b + 1;
            this->vm().check_state(n);
            for (int i = b; i <= c; ++i)
            {
                this->vm().push_value(i);
            }
            this->vm().concat(n);
            this->vm().replace(a);
        }

        void eq(InstructionT ins) { return this->compare_impl(ins, LUA_OPEQ); }
        void lt(InstructionT ins) { return this->compare_impl(ins, LUA_OPLT); }
        void le(InstructionT ins) { return this->compare_impl(ins, LUA_OPLE); }

        void not_(InstructionT ins)
        {
            auto [a, b, c] = ins.ABC();
            a++;
            b++;
            this->vm().push_boolean(!this->vm().to_boolean(b));
            this->vm().replace(a);
        }

        void test_set(InstructionT ins)
        {
            auto [a, b, c] = ins.ABC();
            a++;
            b++;
            if (this->vm().to_boolean(b) == (c != 0))   
                this->vm().copy(b, a);
            else
                this->vm().add_PC(1);
        }

        void test(InstructionT ins)
        {
            auto [a, _, c] = ins.ABC();
            a++;
            if (this->vm().to_boolean(a) != (c != 0))
                this->vm().add_PC(1);
        }

        void length(InstructionT ins)
        {
            auto [a, b, _] = ins.ABC();
            a++;
            b++;
            this->vm().len(b);
            this->vm().replace(a);
        }

    };

} // namespace lua
