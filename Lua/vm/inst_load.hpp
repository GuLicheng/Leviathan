#pragma once

namespace lua
{

    template <typename VM, typename InstructionT>
    class LoadInstructionsInterface
    {

        VM& vm() 
        {
            return static_cast<VM&>(*this);
        }

    public:

        // R(A), R(A+1), ..., R(A+B) := nil
        void load_nil(InstructionT ins)
        {
            auto [a, b, _] = ins.ABC();
            a++;
            this->vm().push_nil();
            for (int i = a; i <= a + b; ++i)
                this->vm().copy(-1, i);
            this->vm().pop(1);
        }

        // R(A) := (bool)B; if (C) pc++
        void load_bool(InstructionT ins)
        {
            auto [a, b, c] = ins.ABC();
            a++;
            this->vm().push_boolean(b != 0);
            this->vm().replace(a);
            if (c != 0)
                this->vm().add_PC(1);
        }

        // R(A) := Kst(Bx)
        void load_k(InstructionT ins) 
        {
            auto [a, bx] = ins.ABx();
            a++;
            this->vm().get_const(bx);
            this->vm().replace(a);
        }

        // R(A) := Kst(extra arg)
        void load_kx(InstructionT ins)
        {
            auto [a, _] = ins.ABx();
            a++;
            auto ax = InstructionT(this->vm().fetch()).Ax();
            this->vm().check_state(1);
            this->vm().get_const(ax);
            this->vm().replace(a);
        }

    };

} // namespace lua
