#pragma once


#include <cstddef>
#include <string>
#include <cstdint>
#include <vector>
#include <memory>
// #include <any> // use this as a placehold
#include <variant>

namespace lua
{
    struct Header
    {
        char signature[4];
        char version;
        char format;
        char luac_data[6];
        char cint_size;
        char sizet_size;
        char instruction_size;
        char lua_integer_size;
        char lua_number_size;
        std::int64_t luac_int;
        double luac_num;
    };

    struct Upvalue
    {
        char instack;
        char idx;
    };

    struct LocVar
    {
        std::string var_name;
        std::uint32_t start_PC;
        std::uint32_t end_PC;
    };


    // Some Constant Variable
    struct ConstantVariable
    {
	    static constexpr auto LUA_SIGNATURE    = "\x1bLua";
	    static constexpr auto LUAC_VERSION     = 0x53;
	    static constexpr auto LUAC_FORMAT      = 0;
	    static constexpr auto LUAC_DATA        = "\x19\x93\r\n\x1a\n";
	    static constexpr auto CINT_SIZE        = 4;
	    static constexpr auto CSIZET_SIZE      = 8;
	    static constexpr auto INSTRUCTION_SIZE = 4;
	    static constexpr auto LUA_INTEGER_SIZE = 8;
	    static constexpr auto LUA_NUMBER_SIZE  = 8;
	    static constexpr auto LUAC_INT         = 0x5678;
	    static constexpr auto LUAC_NUM         = 370.5;

        static constexpr auto TAG_NIL       = 0x00;
        static constexpr auto TAG_BOOLEAN   = 0x01;
        static constexpr auto TAG_NUMBER    = 0x03;
        static constexpr auto TAG_INTEGER   = 0x13;
        static constexpr auto TAG_SHORT_STR = 0x04;
        static constexpr auto TAG_LONG_STR  = 0x14;

        // We use an union to represent the NIL, boolean, number, integer, short/long string
        using value_type = std::variant<std::nullptr_t, bool, double, std::int64_t, std::string>;

    };

    struct Prototype
    {
        std::string source;
        std::uint32_t line_defined;
        std::uint32_t last_line_defined;
        char num_params;
        char is_vararg;
        char max_stack_size;
        std::vector<std::uint32_t> code;
        // std::vector<std::any> constants;
        std::vector<class LuaValue> constants;
        std::vector<Upvalue> upvalues; 
        // std::vector<Prototype*> protos;
        std::vector<std::shared_ptr<Prototype>> protos;
        std::vector<uint32_t> line_info;
        std::vector<LocVar> loc_vars;
        std::vector<std::string> upvalue_names;
    };

    struct BinaryChunk
    {
        Header header;
        char size_upvalues;
        std::shared_ptr<Prototype> main_func;
    };


} // namespace lua

