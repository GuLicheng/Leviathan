#include "../go2cpp.hpp"

namespace lua
{

    template <typename VM, typename InstructionT>
    class MiscInstructionsInterface
    {

        VM& vm() 
        {
            return static_cast<VM&>(*this);
        }

    public:

        // MOVE
        void move(InstructionT ins)
        {
            auto [a, b, _] = ins.ABC();
            a++;
            b++;
            this->vm().copy(b, a);
        }

        // JMP
        void jmp(InstructionT ins)
        {
            auto [a, sBx] = ins.AsBx();
            this->vm().add_PC(sBx);
            if (a != 0)
                panic("TODO!");
        }

    };

} // namespace lua


/*
    C++23 version:

    struct MiscInstructionsInterface
    {
        void move(this auto& self, InstructionT ins)
        {
            auto [a, b, _] = ins.ABC();
            a++;
            b++;
            self.copy(b, a);
        }
    };

    class LuaVM : public MiscInstructionsInterface { };

*/
