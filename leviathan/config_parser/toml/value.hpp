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
