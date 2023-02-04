#pragma once

#include "../api/constants.hpp"
#include "../number/parser.hpp"
#include "../number/math.hpp"
#include "lua_table.hpp"

#include <variant> 
#include <cstdint>
#include <string>
#include <utility>
#include <iosfwd>

namespace lua
{

    class LuaValue;

    using LuaTable = LuaTableT<LuaValue>;

    template <typename T, typename... Ts>
    struct is_one_of : std::disjunction<std::is_same<std::remove_cvref_t<T>, Ts>...> { };

    template <typename T>
    struct is_lua_value : is_one_of<T, std::nullptr_t, bool, int64, float64, std::string, LuaTable> { };

    template <typename L, typename R>
    struct lua_both_arithmetic
    {
        static constexpr bool value = 
            std::is_same_v<float64, L> || 
            std::is_same_v<int64, L> || 
            std::is_same_v<float64, R> || 
            std::is_same_v<int64, R>;
    };

    template <typename L, typename R>
    struct lua_comparable_eq 
    {
        static constexpr bool value = 
            std::is_same_v<L, R> || 
            std::conjunction_v<std::is_arithmetic<L>, std::is_arithmetic<R>>;
    };

    template <typename L, typename R>
    struct lua_comparable_lt
    {
        static constexpr bool value = 
            (std::is_same_v<L, std::string> && std::is_same_v<R, std::string>) || 
            std::conjunction_v<std::is_arithmetic<L>, std::is_arithmetic<R>>;
    };

    template <typename L, typename R>
    struct lua_comparable_le : lua_comparable_lt<L, R> { };

    class LuaValue
    {
        using value_type = std::variant<std::nullptr_t, bool, int64, float64, std::string, LuaTable>;

        value_type data;

    public:

        LuaValue() = default;

        template <typename T>
            requires is_lua_value<T>::value
        /* explicit */ LuaValue(T&& value) : data((T&&) value) { }

        template <typename T>
        bool is() const
        {
            return std::holds_alternative<T>(data);
        }

        template <typename T>
        T& as()
        {
            return std::get<T>(data);
        }

        template <typename T>
        const T& as() const
        {
            return std::get<T>(data);
        }

        LuaType lua_type() const
        {
            using enum LuaType;
            switch (data.index())
            {
                // !val.valueless_by_exception();
                case 0: return LUA_TNIL;
                case 1: return LUA_TBOOLEAN;
                case 2: return LUA_TNUMBER;
                case 3: return LUA_TNUMBER;
                case 4: return LUA_TSTRING;
                case 5: return LUA_TTABLE;
                default: std::unreachable();  // assert val.index() != std::variant_npos 
            }
        }

        friend std::ostream& operator<<(std::ostream& os, const LuaValue& lua_value)
        {
            std::visit([&]<typename T>(const T& v) {
                if constexpr (std::is_same_v<std::nullptr_t, T>)
                    os << "[nullptr/nil]";
                else if constexpr (std::is_same_v<bool, T>)
                    os << (v ? "[true]" : "[false]"); 
                else if constexpr (std::is_same_v<std::string, T>)
                    os << "[\"" << v << "\"]";
                else
                    os << '[' << v << ']';
            }, lua_value.data);
            return os;
        }

        friend bool operator==(const LuaValue& lhs, const LuaValue& rhs)
        {
            return std::visit([]<typename L, typename R>(const L& x, const R& y) -> bool {
                if constexpr (lua_comparable_eq<L, R>::value)
                    return x == y;
                else
                    return false;
            }, lhs.data, rhs.data);
        }

        friend bool operator<(const LuaValue& lhs, const LuaValue& rhs)
        {
            return std::visit([]<typename L, typename R>(const L& x, const R& y) -> bool {
                if constexpr (lua_comparable_lt<L, R>::value)
                    return x < y;
                else
                    panic("comparison error!");
            }, lhs.data, rhs.data);
        }

        friend bool operator<=(const LuaValue& lhs, const LuaValue& rhs)
        {
            return std::visit([]<typename L, typename R>(const L& x, const R& y) -> bool {
                if constexpr (lua_comparable_le<L, R>::value)
                    return x <= y;
                else
                    panic("comparison error!");
            }, lhs.data, rhs.data);
        }


        std::size_t hash_code() const
        {
            return std::visit([]<typename T>(const T& x) -> std::size_t {
                if constexpr (is_one_of<T, std::nullptr_t, bool, int64, float64, std::string>::value)
                    return std::hash<T>()(x);
                else    
                    panic("Table is not hashable.");
            }, this->data);
        }

    };

    LuaType type_of(const LuaValue& val)
    {
        return val.lua_type();
    }

    bool convert_to_boolean(const LuaValue& val)
    {
        if (val.is<std::nullptr_t>())
            return false;
        if (val.is<bool>())
            return val.as<bool>();
        return true;
    }

    Expected<float64> convert_to_float(const LuaValue& val)
    {
        if (val.is<float64>())
            return { val.as<float64>() };
        if (val.is<int64>())
            return { static_cast<float64>(val.as<int64>()) };
        if (val.is<std::string>())
            return parse_float(val.as<std::string>());
        return UnExpected("Error LuaType(cannot convert lua value from {TODO: } to float).");
    }

    Expected<int64> string_to_integer(const std::string& s)
    {
        if (auto epi = parse_integer(s); epi)
            return epi;
        if (auto epf = parse_float(s); epf)
            return epf;
        return UnExpected("The string cannot convert to integer).");
    }

    Expected<int64> convert_to_integer(const LuaValue& val)
    {
        if (val.is<int64>())
            return { val.as<int64>() };
        if (val.is<float64>())
            return float_to_integer(val.as<float64>());
        if (val.is<std::string>())
            return string_to_integer(val.as<std::string>());
        return UnExpected("Error LuaType(cannot convert lua value from {TODO: } to integer).");
    }


}


