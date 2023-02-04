#pragma once

/*
    nil                         -> nil 
    true/false                  -> boolean
    3.14                        -> number
    "Hello World"               -> string
    {}                          -> table
    print                       -> function
    coroutine.create(print)     -> thread
    io.stdin                    -> userdata
*/

namespace lua
{
    /* basic types */
    enum class LuaType {
        LUA_TNONE = -1,
        LUA_TNIL,
        LUA_TBOOLEAN,
        LUA_TLIGHTUSERDATA,
        LUA_TNUMBER,
        LUA_TSTRING,
        LUA_TTABLE,
        LUA_TFUNCTION,
        LUA_TUSERDATA,
        LUA_TTHREAD,
    };


    /* arithmetic functions */
    enum class LuaArithmeticOp {
        LUA_OPADD,         // +
        LUA_OPSUB,         // -
        LUA_OPMUL,         // *
        LUA_OPMOD,         // %
        LUA_OPPOW,         // ^
        LUA_OPDIV,         // /
        LUA_OPIDIV,        // //
        LUA_OPBAND,        // &
        LUA_OPBOR,         // |
        LUA_OPBXOR,        // ~
        LUA_OPSHL,         // <<
        LUA_OPSHR,         // >>
        LUA_OPUNM,         // -
        LUA_OPBNOT,        // ~
    };

    /* comparison functions */
    enum class LuaComparisonOp {
        LUA_OPEQ,        // ==
        LUA_OPLT,        // <
        LUA_OPLE,        // <=
    };




} // namespace lua




