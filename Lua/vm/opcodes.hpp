#pragma once

#include "../api/constants.hpp"
#include "../api/lua_vm.hpp"
#include "instruction.hpp"
#include <string_view>
#include <functional>

namespace lua
{

    enum class OpMode : char
    {
        IABC,         // [  B:9  ][  C:9  ][ A:8  ][OP:6]
        IABx,         // [      Bx:18     ][ A:8  ][OP:6]
        IAsBx,        // [     sBx:18     ][ A:8  ][OP:6]
        IAx,          // [           Ax:26        ][OP:6]
    };

    enum class OpCode : char
    {
	    OP_MOVE, 
	    OP_LOADK,
	    OP_LOADKX,
	    OP_LOADBOOL,
	    OP_LOADNIL,
	    OP_GETUPVAL,
	    OP_GETTABUP,
	    OP_GETTABLE,
	    OP_SETTABUP,
	    OP_SETUPVAL,
	    OP_SETTABLE,
	    OP_NEWTABLE,
	    OP_SELF,
	    OP_ADD,
	    OP_SUB,
	    OP_MUL,
	    OP_MOD,
	    OP_POW,
	    OP_DIV,
	    OP_IDIV,
	    OP_BAND,
	    OP_BOR,
	    OP_BXOR,
	    OP_SHL,
	    OP_SHR,
	    OP_UNM,
	    OP_BNOT,
	    OP_NOT,
	    OP_LEN,
	    OP_CONCAT,
	    OP_JMP,
	    OP_EQ,
	    OP_LT,
	    OP_LE,
	    OP_TEST,
	    OP_TESTSET,
	    OP_CALL,
	    OP_TAILCALL,
	    OP_RETURN,
	    OP_FORLOOP,
	    OP_FORPREP,
	    OP_TFORCALL,
	    OP_TFORLOOP,
	    OP_SETLIST,
	    OP_CLOSURE,
	    OP_VARARG,
	    OP_EXTRAARG,
    };

    enum class OpArgMask : char
    {
        OpArgN,        // argument is not used
        OpArgU,        // argument is used
        OpArgR,        // argument is a register or a jump offset
        OpArgK,        // argument is a constant or register/constant
    };


    struct Opcode
    {
        char test_flag;         // operator is a test (next instruction must be a jump)
        char set_a_flag;        // instruction set register A
        OpArgMask arg_b_mode;   // B arg mode
        OpArgMask arc_c_mode;   // C arg mode
        OpMode op_mode;         // op mode
        std::string_view name;
        // std::function<void(LuaVM&, Instruction)> action;
        void (LuaVM::*action)(Instruction);
    };

    inline const Opcode opcodes[] = {
        { 0, 1, OpArgMask::OpArgR, OpArgMask::OpArgN, OpMode::IABC /* */, "MOVE    ", &LuaVM::move      }, // R(A) := R(B)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgN, OpMode::IABx /* */, "LOADK   ", &LuaVM::load_k    }, // R(A) := Kst(Bx)
        { 0, 1, OpArgMask::OpArgN, OpArgMask::OpArgN, OpMode::IABx /* */, "LOADKX  ", &LuaVM::load_kx   }, // R(A) := Kst(extra arg)
        { 0, 1, OpArgMask::OpArgU, OpArgMask::OpArgU, OpMode::IABC /* */, "LOADBOOL", &LuaVM::load_bool }, // R(A) := (bool)B; if (C) pc++
        { 0, 1, OpArgMask::OpArgU, OpArgMask::OpArgN, OpMode::IABC /* */, "LOADNIL ", &LuaVM::load_nil  }, // R(A), R(A+1), ..., R(A+B) := nil
        { 0, 1, OpArgMask::OpArgU, OpArgMask::OpArgN, OpMode::IABC /* */, "GETUPVAL", {} }, // R(A) := UpValue[B]
        { 0, 1, OpArgMask::OpArgU, OpArgMask::OpArgK, OpMode::IABC /* */, "GETTABUP", {} }, // R(A) := UpValue[B][RK(C)]
        { 0, 1, OpArgMask::OpArgR, OpArgMask::OpArgK, OpMode::IABC /* */, "GETTABLE", &LuaVM::get_table }, // R(A) := R(B)[RK(C)]
        { 0, 0, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "SETTABUP", {} }, // UpValue[A][RK(B)] := RK(C)
        { 0, 0, OpArgMask::OpArgU, OpArgMask::OpArgN, OpMode::IABC /* */, "SETUPVAL", {} }, // UpValue[B] := R(A)
        { 0, 0, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "SETTABLE", &LuaVM::set_table}, // R(A)[RK(B)] := RK(C)
        { 0, 1, OpArgMask::OpArgU, OpArgMask::OpArgU, OpMode::IABC /* */, "NEWTABLE", &LuaVM::table_operation::new_table }, // R(A) := {} (size = B,C)
        { 0, 1, OpArgMask::OpArgR, OpArgMask::OpArgK, OpMode::IABC /* */, "SELF    ", {} }, // R(A+1) := R(B); R(A) := R(B)[RK(C)]
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "ADD     ", &LuaVM::add }, // R(A) := RK(B) + RK(C)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "SUB     ", &LuaVM::sub }, // R(A) := RK(B) - RK(C)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "MUL     ", &LuaVM::mul }, // R(A) := RK(B) * RK(C)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "MOD     ", &LuaVM::mod }, // R(A) := RK(B) % RK(C)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "POW     ", &LuaVM::pow }, // R(A) := RK(B) ^ RK(C)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "DIV     ", &LuaVM::div }, // R(A) := RK(B) / RK(C)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "IDIV    ", &LuaVM::idiv }, // R(A) := RK(B) // RK(C)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "BAND    ", &LuaVM::band }, // R(A) := RK(B) & RK(C)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "BOR     ", &LuaVM::bor }, // R(A) := RK(B) | RK(C)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "BXOR    ", &LuaVM::bxor }, // R(A) := RK(B) ~ RK(C)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "SHL     ", &LuaVM::shl }, // R(A) := RK(B) << RK(C)
        { 0, 1, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "SHR     ", &LuaVM::shr }, // R(A) := RK(B) >> RK(C)
        { 0, 1, OpArgMask::OpArgR, OpArgMask::OpArgN, OpMode::IABC /* */, "UNM     ", &LuaVM::unm }, // R(A) := -R(B)
        { 0, 1, OpArgMask::OpArgR, OpArgMask::OpArgN, OpMode::IABC /* */, "BNOT    ", &LuaVM::bnot }, // R(A) := ~R(B)
        { 0, 1, OpArgMask::OpArgR, OpArgMask::OpArgN, OpMode::IABC /* */, "NOT     ", &LuaVM::not_ }, // R(A) := not R(B)
        { 0, 1, OpArgMask::OpArgR, OpArgMask::OpArgN, OpMode::IABC /* */, "LEN     ", &LuaVM::length }, // R(A) := length of R(B)
        { 0, 1, OpArgMask::OpArgR, OpArgMask::OpArgR, OpMode::IABC /* */, "CONCAT  ", &LuaVM::concats }, // R(A) := R(B).. ... ..R(C)
        { 0, 0, OpArgMask::OpArgR, OpArgMask::OpArgN, OpMode::IAsBx /**/, "JMP     ", &LuaVM::jmp }, // pc+=sBx; if (A) close all upvalues >= R(A - 1)
        { 1, 0, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "EQ      ", &LuaVM::eq }, // if ((RK(B) == RK(C)) ~= A) then pc++
        { 1, 0, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "LT      ", &LuaVM::lt }, // if ((RK(B) <  RK(C)) ~= A) then pc++
        { 1, 0, OpArgMask::OpArgK, OpArgMask::OpArgK, OpMode::IABC /* */, "LE      ", &LuaVM::le }, // if ((RK(B) <= RK(C)) ~= A) then pc++
        { 1, 0, OpArgMask::OpArgN, OpArgMask::OpArgU, OpMode::IABC /* */, "TEST    ", &LuaVM::test }, // if not (R(A) <=> C) then pc++
        { 1, 1, OpArgMask::OpArgR, OpArgMask::OpArgU, OpMode::IABC /* */, "TESTSET ", &LuaVM::test_set }, // if (R(B) <=> C) then R(A) := R(B) else pc++
        { 0, 1, OpArgMask::OpArgU, OpArgMask::OpArgU, OpMode::IABC /* */, "CALL    ", {} }, // R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1))
        { 0, 1, OpArgMask::OpArgU, OpArgMask::OpArgU, OpMode::IABC /* */, "TAILCALL", {} }, // return R(A)(R(A+1), ... ,R(A+B-1))
        { 0, 0, OpArgMask::OpArgU, OpArgMask::OpArgN, OpMode::IABC /* */, "RETURN  ", {} }, // return R(A), ... ,R(A+B-2)
        { 0, 1, OpArgMask::OpArgR, OpArgMask::OpArgN, OpMode::IAsBx /**/, "FORLOOP ", &LuaVM::for_loop }, // R(A)+=R(A+2); if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }
        { 0, 1, OpArgMask::OpArgR, OpArgMask::OpArgN, OpMode::IAsBx /**/, "FORPREP ", &LuaVM::for_pred }, // R(A)-=R(A+2); pc+=sBx
        { 0, 0, OpArgMask::OpArgN, OpArgMask::OpArgU, OpMode::IABC /* */, "TFORCALL", {} }, // R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2));
        { 0, 1, OpArgMask::OpArgR, OpArgMask::OpArgN, OpMode::IAsBx /**/, "TFORLOOP", {} }, // if R(A+1) ~= nil then { R(A)=R(A+1); pc += sBx }
        { 0, 0, OpArgMask::OpArgU, OpArgMask::OpArgU, OpMode::IABC /* */, "SETLIST ", &LuaVM::set_list }, // R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B
        { 0, 1, OpArgMask::OpArgU, OpArgMask::OpArgN, OpMode::IABx /* */, "CLOSURE ", {} }, // R(A) := closure(KPROTO[Bx])
        { 0, 1, OpArgMask::OpArgU, OpArgMask::OpArgN, OpMode::IABC /* */, "VARARG  ", {} }, // R(A), R(A+1), ..., R(A+B-2) = vararg
        { 0, 0, OpArgMask::OpArgU, OpArgMask::OpArgU, OpMode::IAx /*  */, "EXTRAARG", {} }, // extra (larger) argument for previous opcode
    };

} // namespace lua

