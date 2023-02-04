#pragma once

namespace lua
{
    struct Instruction;
    
} // namespace lua

#include "../vm/inst_load.hpp"
#include "../vm/inst_misc.hpp"
#include "../vm/inst_for.hpp"
#include "../vm/inst_operators.hpp"
#include "../vm/inst_table.hpp"
#include "../state/lua_state.hpp"

#include "../vm/instruction.hpp"

namespace lua
{
    
    class LuaVM : public LuaState, 
        public LoadInstructionsInterface<LuaVM, Instruction>,
        public MiscInstructionsInterface<LuaVM, Instruction>,
        public OperatorInstructionsInterface<LuaVM, Instruction>,
        public ForInstructionsInterface<LuaVM, Instruction>,
        public TableInstructionsInterface<LuaVM, Instruction>
    { 

    public:
        
        using LuaState::LuaState;

        using table_operation = TableInstructionsInterface<LuaVM, Instruction>;

    };


} // namespace lua


