#pragma once

#include "chunk.hpp"
#include "../go2cpp.hpp"
#include "../state/lua_value.hpp"
#include <assert.h>
#include <exception>
#include <string_view>
#include <bit>



namespace lua
{

    static_assert(std::endian::native == std::endian::little);

    struct Reader
    {
        // The inner string type in some programming languages(C#, Java, Python) is equivalent to const char* in C.
        // So we use ori_data to save origin string. The slice operation in string_view is much
        // more efficient than string.
        std::string ori_data; 
        std::string_view data;

        template <typename T>
        T read_basic_type()
        {
            constexpr auto offset = sizeof(T);
            T i = *reinterpret_cast<const T*>(this->data.data());
            this->data = this->data.substr(offset);
            return i;
        }

        auto read_bytes(std::uint32_t n)
        {
            auto bytes = this->data.substr(0, n);
            this->data = this->data.substr(n);
            return bytes;
        }

        template <typename T, typename Callback, typename... Args>
        std::vector<T> read_prototype_members(Callback callable, const Args&... args)
        {
            std::vector<T> result(this->read_uint32());
            for (std::size_t i = 0; i < result.size(); ++i)
                result[i] = callable(args...);
            return result;
        }


    public:

        Reader(std::string context)
            : ori_data(std::move(context)), data(ori_data) { }

        auto read_byte() 
        {
            return this->read_basic_type<char>();
        }
        
        auto read_uint32()
        {
            return this->read_basic_type<std::uint32_t>();
        }

        auto read_uint64()
        {
            return this->read_basic_type<std::uint64_t>();
        }

        auto read_lua_integer() 
        {
            return static_cast<std::int64_t>(this->read_uint64());
        }

        auto read_lua_number()
        {
            auto number = this->read_uint64();
            auto floating = *reinterpret_cast<double*>(&number);
            return floating;
        }

        std::string read_string()
        {
            auto size = static_cast<std::uint32_t>(this->read_byte());
            if (size == 0)  // NULL
                return "";

            if (size == 0xFF) // long string
                size = static_cast<std::uint32_t>(this->read_uint64());

            auto bytes = this->read_bytes(size - 1);
            return std::string(bytes);
        }

        void check_header()
        {

            if (std::string(this->read_bytes(4)) != ConstantVariable::LUA_SIGNATURE)
                panic("not a precompiled chunk!");
            if (this->read_byte() != ConstantVariable::LUAC_VERSION)
                panic("version mismatch!");
            if (this->read_byte() != ConstantVariable::LUAC_FORMAT)
                panic("format mismatch!");
            if (std::string(this->read_bytes(6)) != ConstantVariable::LUAC_DATA)
                panic("corrupted!");
            if (this->read_byte() != ConstantVariable::CINT_SIZE)
                panic("int size mismatch!");
            if (this->read_byte() != ConstantVariable::CSIZET_SIZE)    
                panic("size_t mismatch!");
            if (this->read_byte() != ConstantVariable::INSTRUCTION_SIZE)
                panic("instruction size mismatch!");
            if (this->read_byte() != ConstantVariable::LUA_INTEGER_SIZE) 
                panic("lua_Integer size mismatch!");
            if (this->read_byte() != ConstantVariable::LUA_NUMBER_SIZE) 
                panic("lua_Number size mismatch!");
            if (this->read_lua_integer() != ConstantVariable::LUAC_INT) 
                panic("endianness mismatch!");
            if (this->read_lua_number() != ConstantVariable::LUAC_NUM) 
                panic("float format mismatch!");

        }


        // We use smart pointer to manage Prototype
        std::shared_ptr<Prototype> read_proto(const std::string& parent_source) 
        {
            auto source = this->read_string();
            if (source.empty())
                source = parent_source;

            auto proto = std::make_shared<Prototype>();
            proto->source            = source;
            proto->line_defined      = this->read_uint32();
            proto->last_line_defined = this->read_uint32();
            proto->num_params        = this->read_byte();
            proto->is_vararg         = this->read_byte();
            proto->max_stack_size    = this->read_byte();
            proto->code              = this->read_code();
            proto->constants         = this->read_constants();
            proto->upvalues          = this->read_upvalues();
            proto->protos            = this->read_protos(source);
            proto->line_info         = this->read_line_info();
            proto->loc_vars          = this->read_loc_vars();
            proto->upvalue_names     = this->read_upvalue_names();
            
            return proto;
        }

        std::vector<std::string> read_upvalue_names()
        {
            auto op = [this] { return this->read_string(); };
            return this->read_prototype_members<std::string>(op);
        }

        std::vector<LocVar> read_loc_vars()
        {
            auto op = [this] { 
                return LocVar {
                    .var_name = this->read_string(),
                    .start_PC = this->read_uint32(),
                    .end_PC = this->read_uint32()  
                }; 
            };
            return this->read_prototype_members<LocVar>(op);
        }

        std::vector<std::uint32_t> read_line_info()
        {
            auto op = [this] { return this->read_uint32(); };
            return this->read_prototype_members<std::uint32_t>(op);
        }

        std::vector<std::shared_ptr<Prototype>> read_protos(const std::string& parent_source)
        {
            auto op = [this] (const std::string& parent_source) { return this->read_proto(parent_source); };
            return this->read_prototype_members<std::shared_ptr<Prototype>>(op, parent_source);
        }

        std::vector<Upvalue> read_upvalues()
        {
            auto op = [this] () { 
                return Upvalue {
                    .instack = this->read_byte(),
                    .idx = this->read_byte()
                };
            };
            return this->read_prototype_members<Upvalue>(op);
        }

        std::vector<std::uint32_t> read_code()
        {
            auto op = [this] { return this->read_uint32(); };
            return this->read_prototype_members<std::uint32_t>(op);
        }

        std::vector<LuaValue> read_constants()
        {
            // C++20: [] () ->
            // C++23: [] ->
            auto op = [this] -> LuaValue { 
                switch (this->read_byte())
                {
                    case ConstantVariable::TAG_NIL: return nullptr;
                    case ConstantVariable::TAG_BOOLEAN: return this->read_byte() != 0;
                    case ConstantVariable::TAG_INTEGER: return this->read_lua_integer();
                    case ConstantVariable::TAG_NUMBER: return this->read_lua_number();
                    case ConstantVariable::TAG_SHORT_STR:
                    case ConstantVariable::TAG_LONG_STR: return this->read_string();
                    default: panic("corrupted"); // TODO
                }
            };
            return this->read_prototype_members<LuaValue>(op);
        } 

    };


    std::shared_ptr<Prototype> undump(std::string data) 
    {
        Reader reader { data };
        reader.check_header();
        reader.read_byte(); // size upvalues
        return reader.read_proto("");
        // reader := &reader{data}
        // reader.checkHeader()
        // reader.readByte() // size_upvalues
        // return reader.readProto("")
    }

} // namespace lua



