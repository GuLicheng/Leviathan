#pragma once

#include "array.hpp"
#include "table.hpp"
#include "datetime.hpp"
#include "../common.hpp"
#include <leviathan/variable.hpp>

namespace cpp::config::toml
{
    
using cpp::string::string_hash_key_equal;

template <typename T>
using global_allocator = std::allocator<T>;

template <typename T>
struct deleter
{
    static void constexpr operator()(T* p) 
    { 
        std::destroy_at(p);
        global_allocator<T>().deallocate(p, 1);
    };
};

template <size_t N>
struct as_unique_ptr_if_large_than
{
    template <typename U>
    using type = std::conditional_t<(sizeof(U) > N), std::unique_ptr<U, deleter<U>>, U>;

    template <typename T>
    static constexpr auto from_value(T t) 
    {
        if constexpr (sizeof(T) > N)
        {
            auto ptr = global_allocator<T>().allocate(1);
            std::construct_at(ptr, std::move(t));            
            return std::unique_ptr<T, deleter<T>>(ptr, deleter<T>());
        }
        else
        {
            return t;
        }
    }

    template <typename T>
    static constexpr auto to_address(T* t) 
    {
        if constexpr (!meta::specialization_of<std::remove_cv_t<T>, std::unique_ptr>)
        {
            return t;
        }
        else
        {
            return std::to_address(*t);
        }
    }
};

class value;

using boolean = bool;
using floating = double;
using integer = int64_t;
using string = std::basic_string<char, std::char_traits<char>, global_allocator<char>>;
using array = detail::toml_array_base<value, global_allocator<value>>;
using table = detail::toml_table_base<string, value, string_hash_key_equal, global_allocator<std::pair<const string, value>>>;

using toml_value_base = variable<
    as_unique_ptr_if_large_than<16>,
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

} // namespace cpp::config::toml

namespace cpp::toml
{
using namespace ::cpp::config::toml;
}
