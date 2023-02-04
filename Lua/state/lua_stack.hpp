#pragma once

#include "lua_value.hpp"
#include "../go2cpp.hpp"

#include <vector>
#include <assert.h>
#include <memory>
#include <iostream>

namespace lua
{
    // LuaStack will not dynamic expand
    class LuaStack
    {
        friend class LuaState;

        std::vector<LuaValue> slots;
        // int top;  // may unnecessary
    
    public:

        LuaStack(int size) 
        {
            this->slots.reserve(size);
            // this->top = 0;
        }

        void check(std::size_t n)
        {
            this->slots.reserve(n + this->slots.size());
        }        

        void push(const LuaValue& val)
        {
            if (this->slots.size() == this->slots.capacity())
                panic(false && "stack overflow!");
            this->slots.emplace_back(val);            
        }

        void push(LuaValue&& val)
        {
            if (this->slots.size() == this->slots.capacity())
                panic(false && "stack overflow!");
            this->slots.emplace_back(std::move(val));            
        }

        LuaValue pop()
        {
            if (this->slots.size() < 1)
                panic("stack underflow");
            auto val = std::move(this->slots.back());
            this->slots.pop_back();
            return val;
        }

        int abs_index(int idx) const
        {
            return idx >= 0 ? idx : idx + static_cast<int>(this->slots.size()) + 1;
        }

        bool is_valid(int idx) const
        {
            auto abs_idx = this->abs_index(idx);
            return abs_idx > 0 && abs_idx <= this->slots.size();
        }

        LuaValue& get(int idx) 
        {
            // An alternative way is to use std::shared_ptr<LuaValue> as
            // value_type of slots
            static LuaValue default_nil = nullptr;

            auto abs_idx = this->abs_index(idx);
            if (abs_idx > 0 && abs_idx <= this->slots.size())
                return this->slots[abs_idx - 1];
            return default_nil;
        }

        const LuaValue& get(int idx) const 
        {
            return const_cast<LuaStack&>(*this).get(idx);
        }

        void set(int idx, const LuaValue& val)
        {
            auto abs_idx = this->abs_index(idx);
            if (abs_idx > 0 && abs_idx <= this->slots.size())
            {
                this->slots[abs_idx - 1] = val;
                return;
            }
            panic("invalid index");
        }

        void set(int idx, LuaValue&& val)
        {
            auto abs_idx = this->abs_index(idx);
            if (abs_idx > 0 && abs_idx <= this->slots.size())
            {
                this->slots[abs_idx - 1] = std::move(val);
                return;
            }
            panic("invalid index");
        }

        void reverse(int from, int to)
        {
            auto& slots = this->slots;
            for (; from < to;)
            {
                std::swap(slots[from], slots[to]);
                from++;
                to--;
            }
        }


        friend std::ostream& operator<<(std::ostream& os, const LuaStack& s)
        {
            for (const auto& slot : s.slots)
                os << slot;
            return os;
        }


    };

} // namespace lua


