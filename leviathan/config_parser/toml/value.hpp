#pragma once

#include "array.hpp"
#include "table.hpp"
#include "datetime.hpp"
#include <leviathan/config_parser/common.hpp>
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

    using base = toml_value_base;
    using base::base;
    using base::operator=;

    bool is_boolean() const { return this->is<boolean>(); }
    bool is_integer() const { return this->is<integer>(); }
    bool is_floating() const { return this->is<floating>(); }
    bool is_string() const { return this->is<string>(); }
    bool is_array() const { return this->is<array>(); }
    bool is_table() const { return this->is<table>(); }
    bool is_datetime() const { return this->is<datetime>(); }

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

inline constexpr auto make = cpp::cast<value>;

} // namespace cpp::config::toml

namespace cpp
{

template <typename Source>
class type_caster<toml::value, Source, error_policy::exception>
{
public:
    using result_type = toml::value;

    static result_type operator()(const Source& source)
    {
        if constexpr (std::is_same_v<Source, bool>)
        {
            return toml::make_toml<toml::boolean>(source);
        }
        else if constexpr (std::integral<Source>)
        {
            return toml::make_toml<toml::integer>(source);
        }
        else if constexpr (std::is_floating_point_v<Source>)
        {
            return toml::make_toml<toml::floating>(static_cast<toml::floating>(source));
        }
        else if constexpr (cpp::meta::string_like<Source>)
        {
            return toml::make_toml<toml::string>(source);
        }
        else if constexpr (std::same_as<Source, toml::datetime>)
        {
            return toml::make_toml<toml::datetime>(source);
        }
        else if constexpr (std::ranges::range<Source>)
        {
            using ValueType = std::ranges::range_value_t<Source>;

            if constexpr (meta::pair_like<ValueType>)
            {
                using KeyType = std::tuple_element_t<0, ValueType>;
                using KeyTypeCaster = type_caster<toml::string, KeyType, error_policy::exception>;

                using MappedType = std::tuple_element_t<1, ValueType>;
                using MappedTypeCaster = type_caster<toml::value, MappedType, error_policy::exception>;

                auto as_pair = [](const auto& pairlike) static {
                    return std::make_pair(
                        toml::string(std::get<0>(pairlike)), 
                        MappedTypeCaster::operator()(std::get<1>(pairlike))
                    );
                };

                return toml::make_toml<toml::table>(
                    source | std::views::transform(as_pair) | std::ranges::to<toml::table>()
                );
            }
            else
            {
                return toml::make_toml<toml::array>(
                    source | std::views::transform(type_caster<toml::value, ValueType, error_policy::exception>()) | std::ranges::to<toml::array>()
                );
            }
        }
        else
        {
            static_assert(false, "not supported type for toml::value");
        }
    } 
};

}  // namespace cpp
