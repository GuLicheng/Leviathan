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

class value;

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
    {
        return !m_locked;
    }

    bool is_array() const
    {
        return m_locked;
    }
};


} // namespace detail

using boolean = bool;
using floating = double;
using integer = int64_t;
using string = std::string;
using array = detail::toml_array_base<value>;
using table = detail::toml_table_base<string, value>;

// Can we use std::chrono directly?
// YYYY-MM-DDTHH:MM:SS.XXXX+HH:MM
// 1. offset date time: all
// 2. local date time: local date + local time
// 3. local date only
// 4. local time only
class time; // hour + minute + second + nanosecond
class time_offset;  // hour + minute
class date; // YMD year + month + day
class datetime; // above

using toml_value_base = value<
    to_unique_ptr_if_large_than<16>,
    boolean,
    integer,
    floating,
    string,
    array, 
    table,
    datetime
>;

class value : public toml_value_base
{
public:

    using toml_value_base::toml_value_base;
    using toml_value_base::operator=;
};

template <typename Object, typename... Args>
value make_toml(Args&&... args)
{
    return Object((Args&&) args...);
}

struct encoder
{

};

} // namespace leviathan::config::toml

namespace leviathan::toml
{
using namespace ::leviathan::config::toml;
}




