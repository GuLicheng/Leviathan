#pragma once

#include <leviathan/extc++/string.hpp>
#include "number.hpp"
#include "../../variable.hpp"
#include "../../io/file.hpp"

#include <utility>
#include <memory>
#include <vector>
#include <iostream>
#include <compare>
#include <format>
#include <unordered_map>
#include <type_traits>

namespace leviathan::config::json
{

template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

using leviathan::string::string_viewable;
using leviathan::string::string_hash_key_equal;

enum class error_code
{
    ok,
    eof_error,
    uninitialized,
    illegal_string,
    illegal_array,
    illegal_object,
    illegal_number,
    illegal_literal,
    illegal_boolean,
    illegal_unicode,
    error_payload,
    multi_value,
    unknown_character,
};

inline constexpr const char* error_infos[] = {
    "ok",
    "end of file error",
    "uninitialized",
    "illegal_string",
    "illegal_array",
    "illegal_object",
    "illegal_number",
    "illegal_literal",
    "illegal_boolean",
    "illegal_unicode",
    "error_payload",
    "multi_value",
    "unknown_character",
};

constexpr const char* report_error(error_code ec)
{
    return error_infos[static_cast<int>(ec)];
}

class value;

// A better choice is to use Empty class. The value of null is unique, 
// the index in std::variant is enough to indicate it.
using null = std::nullptr_t;   

using string = std::string;
using boolean = bool;
using array = std::vector<value>;

// The std::unordered_map may not efficient, is 
// std::vector<std::pair<const string, value>>
// with a better choice since the object do not contain
// many elements usually.
using object = std::unordered_map<string, value, string_hash_key_equal, string_hash_key_equal>;

using value_base = variable<
    to_unique_ptr_if_large_than<16>, 
    null,
    boolean,
    number,
    string,
    array,
    object,
    error_code  // If some errors happen, return error_code.
>;

// Clang will complain incomplete type but GCC and MSVC are OK.
class value : public value_base
{
public:

    using value_base::value_base;
    using value_base::operator=;

    template <string_viewable... Svs>
    value& operator[](const Svs&... svs) 
    {
        std::string_view views[] = { std::string_view(svs)... };
        value* target = this;
        object default_object = object();

        for (auto sv : views)
        {
            auto& obj = target->as<object>();
            auto it = obj.try_emplace(string(sv), object());
            target = &(it.first->second);
        }

        return *target;
    }

    bool is_integer() const
    {
        return is<number>() 
            && as<number>().is_integer();
    }

    bool is_number() const
    { return is<number>(); }
    
    bool is_boolean() const
    { return is<boolean>(); }

    bool is_null() const
    { return is<null>(); }

    bool is_array() const
    { return is<array>(); }

    bool is_object() const
    { return is<object>(); }

    bool is_string() const
    { return is<string>(); }

    explicit operator bool() const
    { return m_data.index() < std::variant_size_v<value_type> - 1; }

    error_code ec() const
    { 
        auto code = std::get_if<error_code>(&m_data);
        return code ? *code : error_code::ok;
    }
};

template <typename Object, typename... Args>
value make_json(Args&&... args)
{
    return Object((Args&&) args...);
}

}

namespace leviathan::json
{
using namespace ::leviathan::config::json;
}

