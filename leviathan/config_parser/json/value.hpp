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
// a better choice since the object do not contain
// many elements usually.
using object = std::unordered_map<
    string, 
    value, 
    string_hash_key_equal, 
    string_hash_key_equal, 
    global_allocator<std::pair<const string, value>>
>;

using value_base = variable<
    as_unique_ptr_if_large_than<16>,
    null,
    boolean,
    number,
    string,
    array,
    object
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
    };

public:

    using base = value_base;
    using base::base;
    using value_base::operator=;

    // Follow two ctors are used to convert from other std::initializer_list types.
    // For example, `value v = {1, 2, 3};` will call this ctor.
    template <typename T>
    value(T x) : base(cpp::cast<value>(std::move(x)))
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

    template <typename T>
    value& operator=(T x)
    {
        base::operator=(cpp::cast<value>(std::move(x)));
        return *this;
    }

    template <string_viewable StringView>
    value& operator[](const StringView& sv)
    {
        if (this->is_null())
        {
            // Implicitly convert null to object.
            // This is useful when you want to create a new object.
            // For example, `value v; v["key"] = "value";` will create an object.
            this->emplace<object>();
        }
        else if (!this->is_object())
        {
            throw std::runtime_error(std::format("Cannot access '{}' in a non-object value", sv));
        }

        std::string_view key(sv);
        auto [pos, _] = this->as<object>().try_emplace(cpp::cast<string>(key), nullptr);
        return pos->second;
    }

    // TODO: Add operator[] for array.
    // template <std::integral Index>
    // value& operator[](Index index);

    bool is_integer() const
    {
        return is<number>() 
            && as<number>().is_integer();
    }

    bool is_number() const { return is<number>(); }
    bool is_boolean() const { return is<boolean>(); }
    bool is_null() const { return is<null>(); }
    bool is_array() const { return is<array>(); }
    bool is_object() const { return is<object>(); }
    bool is_string() const { return is<string>(); }

    // explicit operator bool() const
    // { return m_data.index() < std::variant_size_v<value_type> - 1; }

    // error_code ec() const
    // { 
    //     auto code = std::get_if<error_code>(&m_data);
    //     return code ? *code : error_code::ok;
    // }

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

inline constexpr auto make = cpp::cast<value>;

}  // namespace cpp::config::json

// Implementation of json::make 
template <typename Source>
class cpp::type_caster<cpp::json::value, Source, cpp::error_policy::exception>
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
                auto as_pair = cpp::views::pair_transform(
                    cpp::cast<cpp::json::string>,
                    cpp::cast<cpp::json::value>
                );

                return json::make_json<json::object>(
                    source | as_pair | std::ranges::to<json::object>()
                );
            }
            else
            {
                return json::make_json<json::array>(
                    source | std::views::transform(cpp::cast<cpp::json::value>) | std::ranges::to<json::array>()
                );
            }
        }
        else
        {
            static_assert(false);
        }
    } 
};


