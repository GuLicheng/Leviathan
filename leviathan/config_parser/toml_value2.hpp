#pragma once

#include "common.hpp"
#include "../value.hpp"

#include <stdexcept>
#include <format>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <utility>

namespace leviathan::config::toml
{

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

class toml_value;

namespace detail
{
    
template <typename T>
class toml_array_base : public std::vector<T>
{
    using base = std::vector<T>;

    bool m_locked = false;  // for fixed array or table array

public:

    toml_array_base() = default;

    template <typename... Args>
    explicit toml_array_base(bool locked, Args&&... args) 
        : base((Args&&) args...), m_locked(locked) { }

    bool is_table_array() const 
    { return !m_locked; }

    bool is_array() const
    { return m_locked; }
};

template <typename K, typename V>
class toml_table_base : public std::unordered_map<K, V, string_hash_key_equal, string_hash_key_equal>
{
    using base = std::unordered_map<K, V, string_hash_key_equal, string_hash_key_equal>;

    bool m_locked = false;   // for table or inline table

    bool m_defined = false;  // for defining a super-table afterward 

public:

    toml_table_base() = default;

    template <typename... Args>
    explicit toml_table_base(bool locked, Args&&... args) 
        : base((Args&&) args...), m_locked(locked) { }

    bool is_inline_table() const 
    { return m_locked; }

    bool is_table() const
    { return !m_locked; }

    bool is_defined() const
    { return m_defined; }

    void define_table()
    { m_defined = true; }
};

} // namespace detail

using toml_boolean = bool;
using toml_float = double;
using toml_integer = int64_t;
using toml_string = std::string;
using toml_array = detail::toml_array_base<toml_value>;
using toml_table = detail::toml_table_base<toml_string, toml_value>;

// Can we use std::chrono directly?
// YYYY-MM-DDTHH:MM:SS.XXXX+HH:MM
// 1. offset date time: all
// 2. local date time: local date + local time
// 3. local date only
// 4. local time only
class toml_datetime;

using toml_value_base = value<
    to_unique_ptr_if_large_than<16>,
    toml_boolean,
    toml_integer,
    toml_float,
    toml_string,
    toml_array, 
    toml_table,
    toml_datetime
>;

class toml_value : toml_value_base
{
public:

    using toml_value_base::toml_value_base;
    using toml_value_base::operator=;
};

template <typename Object, typename... Args>
toml_value make_toml(Args&&... args)
{
    return Object((Args&&) args...);
}

} // namespace leviathan::config::toml

namespace leviathan::toml
{
using namespace ::leviathan::config::toml;
}

namespace leviathan::config::toml::detail
{

struct dump_helper
{
    // TODO: stringify the value 
};

} // namespace leviathan::config::toml::detail

template <typename CharT>
struct std::formatter<leviathan::toml::toml_value, CharT> 
{
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FmtContext>
    typename FmtContext::iterator format(const leviathan::toml::toml_value& value, FmtContext& ctx) const
    {
        leviathan::toml::detail::dump_helper dumper;
        dumper(value);
        return std::ranges::copy(dumper.m_result, ctx.out()).out;
    }   
};

