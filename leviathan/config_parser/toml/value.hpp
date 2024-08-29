#pragma once

#include "array.hpp"
#include "table.hpp"
#include "datetime.hpp"
#include "../common.hpp"
#include "../../variable.hpp"

namespace leviathan::config::toml
{
    
using leviathan::string::string_hash_key_equal;

class value;

using boolean = bool;
using floating = double;
using integer = int64_t;
using string = std::string;
using array = detail::toml_array_base<value>;
using table = detail::toml_table_base<string, value, string_hash_key_equal>;

using toml_value_base = variable<
    to_unique_ptr_if_large_than<16>,
    boolean,
    integer,
    floating,
    string,
    datetime,
    array, 
    table
>;

class value : public toml_value_base
{
public:

    using toml_value_base::toml_value_base;
    using toml_value_base::operator=;

    // bool operator==(const value& rhs) const 
    // {
    //     auto compare = []<typename T1, typename T2>(const T1& x, const T2& y) static
    //     {
    //         if constexpr (std::is_same_v<T1, T2>)
    //         {
    //             return value::accessor()(x) == value::accessor()(y);
    //         }
    //         else
    //         {
    //             return false;
    //         }
    //     };
    //     return std::visit(compare, m_data, rhs.m_data);
    // }
};

template <typename Object, typename... Args>
value make_toml(Args&&... args)
{
    return Object((Args&&) args...);
}

struct toml_parse_error : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

template <typename... Args>
[[noreturn]] void throw_toml_parse_error(std::string_view fmt, Args&&... args)
{
    auto msg = std::vformat(fmt, std::make_format_args(args...));
    throw toml_parse_error(msg);
}

template <typename... Args>
void check_and_throw(bool ok, std::string_view fmt, Args&&... args)
{
    if (!ok)
    {
        auto msg = std::vformat(fmt, std::make_format_args(args...));
        throw toml_parse_error(msg);
    }
}

} // namespace leviathan::config::toml

namespace leviathan::toml
{
using namespace ::leviathan::config::toml;
}
