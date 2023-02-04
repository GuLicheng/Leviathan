#include <iostream>

#include "bintrunk/reader.hpp"
#include "vm/instruction.hpp"
#include "go2cpp.hpp"
#include "api/lua_vm.hpp"
#include "state/lua_state.hpp"

#include <sstream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <utility>



auto print_header(std::shared_ptr<lua::Prototype> f)
{
    auto fun_type = f->line_defined > 0 ? "function" : "main";
    
    std::string vararg_flag = "";
    if (f->is_vararg > 0)   
        vararg_flag = "+";

	fmt.Printf("\n%s <%s:%d,%d> (%d instructions)\n",
		fun_type, f->source.c_str(), f->line_defined, f->last_line_defined, std::size(f->code));

	fmt.Printf("%d%s params, %d slots, %d upvalues, ",
		f->num_params, vararg_flag.c_str(), f->max_stack_size, std::size(f->upvalue_names));

	fmt.Printf("%d locals, %d constants, %d functions\n",
		std::size(f->loc_vars), std::size(f->constants), std::size(f->protos));
    
}

void print_operands(lua::Instruction i)
{
    switch (i.op_mode())
    {
        case lua::OpMode::IABC: 
        {
            auto [a, b, c] = i.ABC();
            fmt.Printf("%d", a);
            if (i.b_mode() != lua::OpArgMask::OpArgN) 
            {
                if (b > 0xFF)
                    fmt.Printf(" %d", (int)(-1-(b&0xFF)));
                else
                    fmt.Printf(" %d", b);
            }
            if (i.c_mode() != lua::OpArgMask::OpArgN)
            {
                if (c > 0xFF)
                    fmt.Printf(" %d", (int)(-1-(c&0xFF)));
                else
                    fmt.Printf(" %d", c);
            }
        } break;

        case lua::OpMode::IABx:
        {
            auto [a, bx] = i.ABx();
            fmt.Printf("%d", a);
            if (i.b_mode() == lua::OpArgMask::OpArgK)
                fmt.Printf(" %d", -1-bx);
            else
                fmt.Printf(" %d", bx);
        } break;

        case lua::OpMode::IAsBx:
        {
            auto [a, sbx] = i.AsBx();
            fmt.Printf("%d %d", a, sbx);
        } break;

        case lua::OpMode::IAx:
        {
            auto ax = i.Ax();
            fmt.Printf("%d", (int)(-1-ax));
        } break;

        default: std::unreachable();
    }
}


auto print_code(std::shared_ptr<lua::Prototype> f)
{

    for (int i = 0; i < f->code.size(); ++i)
    {
        std::string line = "-";
        if (std::size(f->line_info) > 0)
            line = std::to_string(f->line_info[i]);
        auto j = lua::Instruction(f->code[i]);
        // fmt.Printf("\t%d\t[%s]\t0x%08X\n", i+1, line.c_str(), f->code[i]);
        fmt.Printf("\t%d\t[%s]\t%s \t", i+1, line.c_str(), j.op_name().data());
        print_operands(j);
        fmt.Printf("\n");
        
    }
}

auto upvalName(std::shared_ptr<lua::Prototype> f, int idx) -> std::string {
	if (std::size(f->upvalue_names) > 0) 
		return f->upvalue_names[idx];
	return "-";
}

auto constantToString(const auto& k) -> std::string 
{
    std::stringstream ss;
    ss << k;
    return ss.str();
}

auto print_detail(std::shared_ptr<lua::Prototype> f)
{
	fmt.Printf("constants (%d):\n", std::size(f->constants));


	// for i, k := range f.Constants {
	// 	fmt.Printf("\t%d\t%s\n", i+1, constantToString(k))
	// }

    for (int i = 0; i < f->constants.size(); ++i)
        // fmt.Printf("\t%d\t%d\n", i+1, (int)(f->constants[i].index()));
        fmt.Printf("\t%d\t%s\n", i+1, constantToString(f->constants[i]).c_str());

	fmt.Printf("locals (%d):\n", std::size(f->loc_vars));

    for (int i = 0; i < f->loc_vars.size(); ++i)
        fmt.Printf("\t%d\t%s\t%d\t%d\n",
			i, f->loc_vars[i].var_name.c_str(), f->loc_vars[i].start_PC+1, f->loc_vars[i].end_PC+1);

	fmt.Printf("upvalues (%d):\n", std::size(f->upvalues));

    for (int i = 0; i < f->upvalues.size(); ++i)
        fmt.Printf("\t%d\t%s\t%d\t%d\n",
			i, upvalName(f, i).c_str(), f->upvalues[i].instack, f->upvalues[i].idx);
}

void list(std::shared_ptr<lua::Prototype> f)
{
    print_header(f);
    print_code(f);
    print_detail(f);

    for (auto proto : f->protos)
        list(proto);

}

void test1(int argc, const char* argv[])
{
    std::fstream ifs { argv[1], std::ios::in | std::ios::binary };
    std::string context {
        std::istreambuf_iterator<char>(ifs),
        std::istreambuf_iterator<char>()
    };

    auto proto = lua::undump(context);
    list(proto);
}

void print_stack(lua::LuaState& s) 
{

    // std::cout << *s.stack << '\n';

    auto top = s.get_top();
    for (int i = 1; i <= top; ++i) 
    {
        auto t = s.type(i);
        using enum lua::LuaType;
        switch (t)
        {
            case LUA_TBOOLEAN: fmt.Printf("[%s]", s.to_boolean(i) ? "true" : "false"); break;
            case LUA_TNUMBER: fmt.Printf("[%g]", s.to_number(i)); break;
            case LUA_TSTRING: fmt.Printf("[\"%s\"]", s.to_string(i).c_str()); break;
            default: fmt.Printf("[%s]", s.type_name(t).data()); break;
        }
    }

    fmt.Printf("\n");
};

void test2()
{
    auto ls = lua::LuaState(20, nullptr);
    ls.push_boolean(true); print_stack(ls);
    ls.push_integer(10); print_stack(ls);
    ls.push_nil(); print_stack(ls);
    ls.push_string("hello"); print_stack(ls);
    ls.push_value(-4); print_stack(ls);
    ls.replace(3); print_stack(ls);
    ls.set_top(6); print_stack(ls);
    ls.remove(-3); print_stack(ls);
    ls.set_top(-5); print_stack(ls);
}

void test3()
{
    auto ls = lua::LuaState(20, nullptr);
    ls.push_integer(1);
    ls.push_string("2.0");
    ls.push_string("3.0");
    ls.push_number(4.0);

    print_stack(ls);

    ls.arith(lua::LuaArithmeticOp::LUA_OPADD); print_stack(ls);
    ls.arith(lua::LuaArithmeticOp::LUA_OPBNOT); print_stack(ls);
    ls.len(2); print_stack(ls);
    ls.concat(3); print_stack(ls);
    ls.push_boolean(ls.compare(1, 2, lua::LuaComparisonOp::LUA_OPEQ)); print_stack(ls);
}

void lua_main(std::shared_ptr<lua::Prototype> proto)
{
    auto nRegs = (int)proto->max_stack_size;
    auto ls = lua::LuaVM(nRegs + 8, proto);
    ls.set_top(nRegs);
    for (;;)
    {
        auto pc = ls.PC();
        auto inst = lua::Instruction(ls.fetch());
        if (inst.opcode() != (int)lua::OpCode::OP_RETURN)
        {
            inst.execute(ls);
            fmt.Printf("[%02d] %s", pc + 1, inst.op_name().data());
            print_stack(ls);
        }
        else
        {
            break;
        }
    }
}

void test4(int argc, const char* argv[])
{
    if (argc > 1)
    {
        std::fstream ifs { argv[1], std::ios::in | std::ios::binary };
        std::string context {
            std::istreambuf_iterator<char>(ifs),
            std::istreambuf_iterator<char>()
        };

        auto proto = lua::undump(context);
        lua_main(proto);
    }
}

int main(int argc, const char* argv[])
{
    test1(argc, argv);
    // test2();
    // test3();
    test4(argc, argv);
}





