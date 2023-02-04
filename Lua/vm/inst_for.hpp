#pragma once

#include "../api/constants.hpp"

namespace lua
{

    // Just for aligning PDF

    // We use template to avoid the error that cycle reference
    // opcodes.hpp -> instructions.hpp
    // instructions.hpp -> opcodes.hpp
    template <typename VM, typename InstructionT>
    class ForInstructionsInterface
    {

        VM& vm() 
        {
            return static_cast<VM&>(*this);
        }

    public:
        using enum LuaType;
        using enum LuaArithmeticOp;
        using enum LuaComparisonOp;

        void for_pred(InstructionT ins)
        {
            auto [a, sBx] = ins.AsBx();
            a++;
            if (this->vm().type(a) == LUA_TSTRING)
            {
                this->vm().push_number(this->vm().to_number(a));
                this->vm().replace(a);
            }

            if (this->vm().type(a + 1) == LUA_TSTRING)
            {
                this->vm().push_number(this->vm().to_number(a + 1));
                this->vm().replace(a + 1);
            }

            if (this->vm().type(a + 2) == LUA_TSTRING)
            {
                this->vm().push_number(this->vm().to_number(a + 2));
                this->vm().replace(a + 2);
            }
            this->vm().push_value(a);
            this->vm().push_value(a + 2);
            this->vm().arith(LUA_OPSUB);
            this->vm().replace(a);
            this->vm().add_PC(sBx);

        }

        void for_loop(InstructionT ins)
        {
            auto [a, sBx] = ins.AsBx();
            a++;
            this->vm().push_value(a + 2);
            this->vm().push_value(a);
            this->vm().arith(LUA_OPADD);
            this->vm().replace(a);


            auto is_positive_step = this->vm().to_number(a + 2) >= 0;
            if (is_positive_step && this->vm().compare(a, a + 1, LUA_OPLE) || 
                !is_positive_step && this->vm().compare(a + 1, a, LUA_OPLE))
            {
                this->vm().add_PC(sBx);
                this->vm().copy(a, a + 3);
            }

        }

    };

} // namespace lua
