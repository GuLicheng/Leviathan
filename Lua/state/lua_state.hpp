#pragma once

#include "lua_stack.hpp"
#include "../bintrunk/chunk.hpp"
#include "lua_value.hpp"
#include "../number/parser.hpp"
#include "api_arith.hpp"

#include <algorithm>


namespace lua
{
    class LuaState
    {
    public:
        std::shared_ptr<LuaStack> stack;
        std::shared_ptr<Prototype> proto;
        int pc;


        using enum LuaType;


    public:

        LuaState(int stack_size, std::shared_ptr<Prototype> proto) 
            : stack(std::make_shared<LuaStack>(stack_size)), proto(proto), pc(0) { }

        int PC() const
        {
            return this->pc;
        }

        void add_PC(int n) 
        {
            this->pc += n;
        }

        uint32 fetch()
        {
            auto code = this->proto->code[this->pc];
            this->pc++;
            return code;
        }

        void get_const(int idx)
        {
            auto c = this->proto->constants[idx];
            this->stack->push(c);
        }

        void get_RK(int rk)
        {
            if (rk > 0xFF)
                this->get_const(rk & 0xFF);
            else
                this->push_value(rk + 1);
        }

        static std::string_view type_name(LuaType tp)
        {
            switch (tp)
            {
                case LUA_TNONE:     return "no value";
                case LUA_TNIL:      return "nil";
                case LUA_TBOOLEAN:  return "boolean";
                case LUA_TNUMBER:   return "number";
                case LUA_TSTRING:   return "string";
                case LUA_TTABLE:    return "table";
                case LUA_TFUNCTION: return "function";
                case LUA_TTHREAD:   return "thread";
                default:            return "userdata";
            }
        }

        // Return index of  stack
        int get_top() 
        {
            return this->stack->slots.size();
        }

        int abs_index(int idx) const
        {
            return this->stack->abs_index(idx);
        }

        bool check_state(int n)
        {
            this->stack->check(n);
            return true;
        }

        void pop(int n)
        {
            for (int i = 0; i < n; ++i)
                this->stack->pop();
        }

        void copy(int from_idx, int to_idx)
        {
            auto val = this->stack->get(from_idx);
            this->stack->set(to_idx, val);
        }

        void push_value(int idx)
        {
            auto val = this->stack->get(idx);
            this->stack->push(val);
        }

        void replace(int idx)
        {
            auto val = this->stack->pop();
            this->stack->set(idx, val);
        }

        void insert(int idx)
        {
            this->rotate(idx, 1);
        }

        void remove(int idx)
        {
            this->rotate(idx, -1);
            this->pop(1);
        }

        void rotate(int idx, int n)
        {
            // auto rotate_elements = *(this->stack) | std::views::drop(idx)
            // if shift left(n < 0):             
            // std::ranges::rotate(rotate_elements, rotate_elements.begin() + n) 
            // else if shift right(n > 0):
            // std::ranges::rotate(rotate_elements, rotate_elements.end() - n)

            auto t = this->stack->slots.size() - 1;
            auto p = this->stack->abs_index(idx) - 1;
            int m = n >= 0 ? t - n : p - n - 1;

            this->stack->reverse(p, m);
            this->stack->reverse(m + 1, t);
            this->stack->reverse(p, t);
        }

        void set_top(int idx)
        {
            auto new_top = this->stack->abs_index(idx);
            if (new_top < 0)
                panic("stack overflow");
            auto n = (int)this->stack->slots.size() - new_top;
            if (n > 0)
            {
                for (int i = 0; i < n; ++i)
                    this->stack->pop();
            }
            else
            {
                for (int i = 0; i > n; --i)
                    this->stack->push(nullptr);
            }
        }

        void push_nil()
        {
            this->stack->push(nullptr);
        }

        void push_boolean(bool b)
        {
            this->stack->push(b);
        }

        void push_integer(std::int64_t n)
        {
            this->stack->push(n);
        }

        void push_number(double n)
        {
            this->stack->push(n);
        }

        void push_string(std::string s)
        {
            this->stack->push(std::move(s));
        }


        LuaType type(int idx) const
        {
            if (this->stack->is_valid(idx))
                return type_of(this->stack->get(idx));
            return LUA_TNONE;
        }

        bool is_none(int idx) const
        {
            return this->type(idx) == LUA_TNONE;
        }
        
        bool is_nil(int idx) const
        {
            return this->type(idx) == LUA_TNIL;
        }

        bool is_none_or_nil(int idx) const
        {
            return this->type(idx) <= LUA_TNIL;
        } 

        bool is_boolean(int idx) const
        {
            return this->type(idx) == LUA_TBOOLEAN;
        } 

        bool is_table(int idx) const
        {
            return this->type(idx) == LUA_TTABLE;
        } 

        bool is_function(int idx) const
        {
            return this->type(idx) == LUA_TFUNCTION;
        } 

        bool is_thread(int idx) const
        {
            return this->type(idx) == LUA_TTHREAD;
        } 

        bool is_string(int idx) const
        {
            const auto t = this->type(idx);
            return t == LUA_TSTRING || t == LUA_TNUMBER;
        } 

        bool is_integer(int idx) const
        {
            const auto& val = this->stack->get(idx);
            return val.is<std::int64_t>();
        }

        bool is_number(int idx) const
        {
            return this->to_numberX(idx).has_value();
        }

        bool to_boolean(int idx) const
        {
            const auto& val = this->stack->get(idx);
            return convert_to_boolean(val);
        }

        double to_number(int idx) const
        {
            return this->to_numberX(idx).value();
        }

        Expected<float64> to_numberX(int idx) const
        {
            const auto& val = this->stack->get(idx);
            return convert_to_float(val); 
        }

        int64 to_integer(int idx) const
        {
            return this->to_integerX(idx).value();
        }

        Expected<int64> to_integerX(int idx) const 
        {
            const auto& val = this->stack->get(idx);
            return convert_to_integer(val);
        }

        std::string to_string(int idx) 
        {
            return this->to_stringX(idx).value();
        } 

        Expected<std::string> to_stringX(int idx) 
        {
            const auto& val = this->stack->get(idx);
            if (val.is<std::string>())
                return { val.as<std::string>() };
            if (val.is<int64_t>())
            {
                auto s = std::to_string(val.as<int64>());
                this->stack->set(idx, s);
                return { s };
            }
            if (val.is<float64>())
            {
                auto s = std::to_string(val.as<float64>());
                this->stack->set(idx, s);
                return { s };
            }
            return UnExpected("This lua value is not string.");
        }

        void arith(LuaArithmeticOp op)
        {
            using enum LuaArithmeticOp;

            LuaValue a, b;
            b = this->stack->pop();
            if (op != LUA_OPUNM && op != LUA_OPBNOT)
                a = this->stack->pop();
            else
                a = b; 

            const auto& operator_ = operators[static_cast<int>(op)];
            if (auto result = this->arith_impl(a, b, operator_); !result.is<std::nullptr_t>())
                this->stack->push(result);
            else
                panic("Arithmetic error!");

        }

        LuaValue arith_impl(const LuaValue& a, const LuaValue& b, const Operator& op)
        {
            if (!op.float_func)
            {
                if (auto x = convert_to_integer(a); x)
                {
                    if (auto y = convert_to_integer(b); y)
                    {
                        return op.integer_func(x.value(), y.value());
                    }
                }
            }
            else 
            {
                // add, sub, mul, mod, idiv, unm
                if (op.integer_func)
                {
                    if (a.is<int64>() && b.is<int64>())
                    {
                        return op.integer_func(a.as<int64>(), b.as<int64>());
                    }
                }

                if (auto x = convert_to_float(a); x)
                {
                    if (auto y = convert_to_float(b); y)
                    {
                        return op.float_func(x.value(), y.value());
                    }
                }

            }
            return nullptr;
        }

        bool compare(int idx1, int idx2, LuaComparisonOp op) const
        {
            auto& a = this->stack->get(idx1);
            auto& b = this->stack->get(idx2);
            switch (op)
            {
                using enum LuaComparisonOp;
                case LUA_OPEQ: return a == b;
                case LUA_OPLT: return a < b;
                case LUA_OPLE: return a <= b;
                default: std::unreachable();
            }
        }

        void len(int idx)
        {
            const auto& val = this->stack->get(idx);
            if (val.is<std::string>())
                this->stack->push(std::ssize(val.as<std::string>()));
            else if (val.is<LuaTable>())
                this->stack->push(int64(val.as<LuaTable>().len()));
            else
                panic("length error!");
        }

        void concat(int n)
        {
            if (n == 0)
            {
                this->stack->push(std::string(""));
            }
            else if (n >= 2)
            {
                for (int i = 1; i < n; ++i)
                {
                    if (this->is_string(-1) && this->is_string(-2))
                    {
                        auto s1 = this->to_string(-1);
                        auto s2 = this->to_string(-2);
                        this->stack->pop();
                        this->stack->pop();
                        this->stack->push(s2 + s1);
                        continue;
                    }
                    panic("concatenation error!");
                }
            }
            // n == 1 and do nothing
        }

        void create_table(int narr, int nrec)
        {
            this->stack->push(LuaTable(narr, nrec));
        }

        void new_table()
        {
            this->create_table(0, 0);
        }

        LuaType get_table_impl(int idx)
        {
            const auto& table = this->stack->get(idx);
            auto k = this->stack->pop();
            return this->get_table_impl(table, k);
        }

        LuaType get_table_impl(const LuaValue& t, const LuaValue& k)
        {
            if (t.is<LuaTable>())
            {
                const auto& v = t.as<LuaTable>().get(k);
                std::cout << "V: " << v << '\n';
                this->stack->push(v);
                return type_of(v);
            }
            panic("Not a table.");
        }

        LuaType get_field(int idx, const std::string& k) 
        {
            auto t = this->stack->get(idx);
            return this->get_table_impl(t, k);
        }

        LuaType get_i(int idx, int64 i)
        {
            const auto& t = this->stack->get(idx);
            return this->get_table_impl(t, i);
        }

        void set_table_impl(int idx)
        {

            std::cout << "Put\n";
            auto& t = this->stack->get(idx);
            auto v = this->stack->pop();
            auto k = this->stack->pop();
            this->set_table_impl(t, std::move(k), std::move(v));
        }

        void set_table_impl(LuaValue& t, LuaValue k, LuaValue v)
        {
            if (t.is<LuaTable>())
                t.as<LuaTable>().put(std::move(k), std::move(v));
            else
                panic("not a table");
        }

        void set_field(int idx, const std::string& k)
        {
            auto& t = this->stack->get(idx);
            auto v = this->stack->pop();
            this->set_table_impl(t, k, std::move(v));
        }

        void set_i(int idx, int64 i)
        {
            auto& t = this->stack->get(idx);
            auto v = this->stack->pop();
            this->set_table_impl(t, i, std::move(v));
        }

    };

} // namespace lua

