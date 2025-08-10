#pragma once

#include <leviathan/config_parser/json/number.hpp>
#include <leviathan/allocators/debug_allocator.hpp>
#include <leviathan/extc++/string.hpp>
#include <leviathan/variable.hpp>
#include <leviathan/variable.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/type_caster.hpp>

#include <utility>
#include <memory>
#include <vector>
#include <expected>
#include <optional>
#include <iostream>
#include <compare>
#include <format>
#include <unordered_map>
#include <type_traits>

namespace cpp::config::json
{

template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

using cpp::string::string_viewable;
using cpp::string::string_hash_key_equal;

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

template <typename T>
using global_allocator = std::allocator<T>;
// using global_allocator = cpp::alloc::debug_allocator<T>;

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

// A better choice is to use Empty class. The value of null is unique, 
// the index in std::variant is enough to indicate it.
using null = std::nullptr_t;   

using string = std::basic_string<char, std::char_traits<char>, global_allocator<char>>;
using boolean = bool;
using array = std::vector<value, global_allocator<value>>;

// The std::unordered_map may not efficient, is 
// std::vector<std::pair<const string, value>>
// with a better choice since the object do not contain
// many elements usually.
using object = std::unordered_map<string, value, string_hash_key_equal, string_hash_key_equal, global_allocator<std::pair<const string, value>>>;

using value_base = variable<
    as_unique_ptr_if_large_than<16>,
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
    static constexpr const char* value_type_names[] = 
    {
        "null",
        "boolean",
        "number",
        "string",
        "array",
        "object",
        "error_code"
    };

public:

    using base = value_base;
    using base::base;
    using value_base::operator=;

    template <typename T>
    value(T x) : base(cpp::type_caster<value, T>()(std::move(x)))
    { }

    value(std::initializer_list<value> init) 
    {
        // Check type of each item in the initializer list.
        bool is_object = std::ranges::all_of(init, [](const value& item) {
            return item.is_array() 
                && item.as<array>().size() == 2 
                && item.as<array>()[0].is_string();
        });

        if (is_object)
        {
            this->emplace<object>();

            for (auto& item : init)
            {
                this->as<object>().try_emplace(
                    std::move(item.as<array>()[0].as<string>()), 
                    const_cast<value&&>(std::move(item.as<array>()[1]))                      
                );
            }
        }
        else
        {
            this->emplace<array>();

            for (auto& item : init)
            {
                this->as<array>().emplace_back(
                    const_cast<value&&>(std::move(item))
                );
            }
        }
    }

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

    const char* type_name() const
    {
        return value_type_names[m_data.index()];
    }
};

template <typename Object, typename... Args>
value make_json(Args&&... args)
{
    return Object((Args&&) args...);
}

inline constexpr simple_caster<value> make;

}  // namespace cpp::config::json

namespace cpp
{

template <typename Source>
class type_caster<json::value, Source, error_policy::exception>
{
public:
    using result_type = json::value;

    static result_type operator()(const Source& source)
    {
        if constexpr (std::is_same_v<Source, bool>)
        {
            return json::make_json<json::boolean>(source);
        }
        else if constexpr (std::is_same_v<Source, std::nullptr_t>)
        {
            return json::make_json<json::null>();
        }
        else if constexpr (meta::arithmetic<Source>)
        {
            return json::make_json<json::number>(source);
        }
        else if constexpr (meta::string_like<Source>)
        {
            return json::make_json<json::string>(source);
        }
        else if constexpr (std::ranges::range<Source>)
        {
            using ValueType = std::ranges::range_value_t<Source>;

            if constexpr (meta::pair_like<ValueType>)
            {
                using KeyType = std::tuple_element_t<0, ValueType>;
                using KeyTypeCaster = type_caster<json::string, KeyType, error_policy::exception>;

                using MappedType = std::tuple_element_t<1, ValueType>;
                using MappedTypeCaster = type_caster<json::value, MappedType, error_policy::exception>;

                auto as_pair = [](const auto& pairlike) static {
                    return std::make_pair(
                        json::string(std::get<0>(pairlike)), 
                        MappedTypeCaster::operator()(std::get<1>(pairlike))
                    );
                };

                return json::make_json<json::object>(
                    source | std::views::transform(as_pair) | std::ranges::to<json::object>()
                );
            }
            else
            {
                using ValueTypeCaster = type_caster<json::value, ValueType, error_policy::exception>;
                
                return json::make_json<json::array>(
                    source | std::views::transform(ValueTypeCaster()) | std::ranges::to<json::array>()
                );
            }
        }
        else
        {
            static_assert(false);
        }
    } 
};

}
