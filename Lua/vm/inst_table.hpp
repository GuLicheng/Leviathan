#pragma once

#include "../api/constants.hpp"
#include "go2cpp.hpp"

namespace lua
{

    int Int2fb(int x)
    {
        int e = 0;
        if (x < 8) 
            return x;
        for (; x >= (8 << 4);)
        {
            x = (x + 0xF) >> 4; // x = ceil(x / 16)
            e += 4;
        }

        for (; x >= (8 << 1);)
        {
            x = (x + 1) >> 1;
            e++;
        }
        return ((e + 1) << 3) | (x - 8);
    }

    int Fb2int(int x)
    {
        if (x < 8)
            return x;
        else    
            return ((x & 7) + 8) << ((x >> 3) - 1);    
    }


    template <typename VM, typename InstructionT>
    class TableInstructionsInterface
    {

        VM& vm() 
        {
            return static_cast<VM&>(*this);
        }

        constexpr static auto LFIELDS_PER_FLUSH = 50;    

    public:
 
        void new_table(InstructionT ins)
        {
            auto [a, b, c] = ins.ABC();
            a++;
            this->vm().create_table(Fb2int(b), Fb2int(c));
            this->vm().replace(a);
        }

        void get_table(InstructionT ins)
        {
            auto [a, b, c] = ins.ABC();
            a++;
            b++;
            this->vm().get_RK(c);
            this->vm().get_table_impl(b);
            std::cout << "Stack is: " << *this->vm().stack << '\n';
            this->vm().replace(a);
            exit(0);
        }

        void set_table(InstructionT ins)
        {
            auto [a, b, c] = ins.ABC();
            a++;
            this->vm().get_RK(b);
            this->vm().get_RK(c);
            this->vm().set_table_impl(a);
        }

        void set_list(InstructionT ins)
        {
            auto [a, b, c] = ins.ABC();
            a++;
            if (c > 0)
            {
                c--;
            }
            else
            {
                c = InstructionT(this->vm().fetch()).Ax();
            }

            auto idx = int64(c * LFIELDS_PER_FLUSH);
            for (int j = 1; j < b; ++j)
            {
                idx++;
                this->vm().push_value(a + j);
                this->vm().set_i(a, idx);
            }
        }

    };

} // namespace lua
