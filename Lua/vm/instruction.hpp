#pragma once

#include "opcodes.hpp"
#include <cstdint>
#include <array>

namespace lua
{

    struct Instruction
    {
        static constexpr auto MAXARG_Bx = (1 << 18) - 1;      // 262143
        static constexpr auto MAXARG_sBx = MAXARG_Bx >> 1;    // 131071

        std::uint32_t ins;

        // Make it implicit
        Instruction(std::uint32_t instruction) : ins(instruction) { }

        int opcode() const
        {
            // 3F: 0000 0000 0000 0000 0000 0000 0011 1111 -- last 6 bits
            return static_cast<int>(this->ins & 0x3F);
        }

        // IABC,   [  B:9  ][  C:9  ][ A:8  ][OP:6]
        std::array<int, 3> ABC() const
        {
            auto a = static_cast<int>(this->ins >> 6 & 0xFF);
            auto c = static_cast<int>(this->ins >> 14 & 0x1FF);
            auto b = static_cast<int>(this->ins >> 23 & 0x1FF); // ?

            // std::cout << "The bit is " << static_cast<int>(((ins >> 22) & 1) == 1) << "\t";
            // std::cout  << ins << " (a, b, c) = (" << a << ", " << b << ", " << c << ")   "; 
            return { a, b, c };
        }

        // IABx,   [      Bx:18     ][ A:8  ][OP:6]
        std::array<int, 2> ABx() const
        {
            auto a = static_cast<int>(this->ins >> 6 & 0xFF);
            auto b = static_cast<int>(this->ins >> 14);
            return { a, b }; 
        }

        // IAsBx,  [     sBx:18     ][ A:8  ][OP:6]
        std::array<int, 2> AsBx() const
        {
            auto [a, bx] = ABx();
            return { a, bx - MAXARG_sBx };  // make unsigned to signed 
        }

        // IAx,    [           Ax:26        ][OP:6]
        int Ax()
        {
            return static_cast<int>(this->ins >> 6);
        }

        auto op_name() const
        {
            return opcodes[this->opcode()].name;
        }

        auto op_mode() const
        {
            return opcodes[this->opcode()].op_mode;
        }

        auto b_mode() const 
        {
            return opcodes[this->opcode()].arg_b_mode;
        }

        auto c_mode() const 
        {
            return opcodes[this->opcode()].arc_c_mode;
        }

        void execute(LuaVM& vm)
        {
            auto& action = opcodes[this->opcode()].action;
            if (action)
                std::invoke(action, vm, *this);
            else
                panic("Error Op.");
        }

    };



} // namespace lua


