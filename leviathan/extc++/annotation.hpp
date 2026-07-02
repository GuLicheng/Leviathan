#pragma once

#include <algorithm>
#include <functional>
#include <ranges>
#include <meta>

namespace cpp::derive
{

/**
 * @brief Allow a class or enum type to be formatted with std::format. 
 */
inline constexpr struct { } debug;

/**
 * @brief Allow a class or enum type to be hashed with std::hash.
 */
inline constexpr struct { } hash;

/**
 * @brief Allow a class or enum to be converted into another 
 * type, for example, json::value. Specialize the `cpp::type_caster` for the 
 * target type to implement the conversion.
 */
template <typename T> struct from_t { explicit from_t() = default; };
template <typename T> inline constexpr auto from = from_t<T>{};

/**
 * @brief Allow a class or enum to be converted from another 
 * type, for example, json::value. Specialize the `cpp::type_caster` for the 
 * source type to implement the conversion.
 */
template <typename T> struct into_t { explicit into_t() = default; };
template <typename T> inline constexpr auto into = into_t<T>{};

/**
 * @brief Allow an enum type to support operator| and operator|=, 
 * which is useful for bitmask operations.
 */
inline constexpr struct { } op_pipe;

/**
 * @brief Allow a class to be treated as a tuple-like type, 
 * which make it supported by std::tuple_size, std::tuple_element.
 * You must derive from `cpp::tuple_get_interface` to provide the get 
 * function for each field.
 */
inline constexpr struct { } tuple_like;

}  // namespace cpp::derive

namespace cpp::refl
{

// Any field annotated with [[=skip]] will be ignored in code generation
// When initializing a struct from a tuple, the skipped fields will be 
// initialized with default value or default initializer.
inline constexpr struct { } skip;

inline constexpr struct { } test;

// Any class annotated with [[=range_maker]] will be treated as a range producer, 
// such as derive from followe class
// class SomeInterface { std::ranges::range<R> operator()(); }
inline constexpr struct { } range_maker;

// Any field annotated with [[=value_guard]] will be treated as a choice field, which means that
// when initializing the field, we will try to find an annotation with [[=value_guard]] 
// and use it to check if the value is valid. Such as derive from followe class
// class SomeInterface { bool operator(const auto&); }
inline constexpr struct { } value_guard;

// inline constexpr struct { } skip_serialization;

// inline constexpr struct { } skip_deserialization;

// Rename annotaion
inline constexpr struct { } modify_identifier;

inline constexpr struct { } initializer;

// inline constexpr struct { } parse_annotation;

// Any type annotated with [[=serializer]] will be treated as a serializer, which means that
// when serializing the type, we will use the serializer to convert it into target type.
inline constexpr struct { } serializer;

// Any type annotated with [[=deserializer]] will be treated as a deserializer, which means that
// when deserializing the type, we will use the deserializer to convert it into target
// type. We do not require the result type of the deserializer to be the string type.
// You can deserializer it as any type you want, such as std::string, SomeBase64, etc.
inline constexpr struct { } deserializer;

}  // namespace cpp::refl



namespace cpp::refl
{

template <typename F>
struct callable
{
    F function;

    explicit constexpr callable(F function) : function(std::move(function)) {}

    template <typename Self, typename... Args>
    constexpr auto operator()(this Self&& self, Args&&... args) 
    {
        return std::invoke(((Self&&)self).function, (Args&&)args...);
    }
};

/*
    - lowercase
    - UPPERCASE
    - PascalCase
    - camelCase
    - snake_case (default)
    - SCREAMING_SNAKE_CASE
    - kebab-case
    - SCREAMING-KEBAB-CASE
*/

template <typename F>
struct [[=modify_identifier]] rename_function : callable<F>
{
    using callable<F>::callable;
    using callable<F>::operator();
};

inline constexpr auto shortname = rename_function([](std::string field_name) static 
{
    // assert(!name.empty(), "Name cannot be empty");
    return '-' + std::string(field_name.begin(), field_name.begin() + 1);
});

inline constexpr auto longname = rename_function([](std::string field_name) static 
{
    // assert(!name.empty(), "Name cannot be empty");
    return "--" + std::string(field_name);
});

inline constexpr auto selfname = rename_function([](std::string field_name) static 
{
    return field_name;
});

inline constexpr auto lowercase = rename_function([](std::string field_name) static 
{
    return field_name | std::views::transform(::tolower) | std::ranges::to<std::string>();
});

inline constexpr auto uppercase = rename_function([](std::string field_name) static 
{
    return field_name | std::views::transform(::toupper) | std::ranges::to<std::string>();
});

inline constexpr auto rename = [](std::string_view new_name) static
{
    return rename_function([name=define_static_string(new_name)](auto&&...)  
    {
        return std::string(name);
    });
};

// Follows functions in terms of implementation maybe incorrect
// FIXME: Rust clap-
inline constexpr auto camel_case = rename_function([](std::string field_name) static
{
    std::string out;
    bool upper_next = false;
    for (char c : field_name) {
        if (c == '_') { upper_next = true; continue; }
        if (upper_next && c >= 'a' && c <= 'z')
            out += static_cast<char>(c - ('a' - 'A'));
        else
            out += c;
        upper_next = false;
    }
    return out;
});

inline constexpr auto pascal_case = rename_function([](std::string field_name) static
{
    auto upper_first_character = [](auto&& part) static {
        if (!part.empty()) part.front() = ::toupper(part.front());
        return part;
    };

    return field_name 
         | std::views::split('_') 
         | std::views::transform(upper_first_character)
         | std::views::join
         | std::ranges::to<std::string>();
});

inline constexpr auto kebab_case = rename_function([](std::string field_name) static
{
    return field_name | std::views::transform([](char c) { return c == '_' ? '-' : c; }) | std::ranges::to<std::string>();
});

template <typename F>
struct [[=initializer]] function_value_annotation : callable<F>
{
    using callable<F>::callable;
    using callable<F>::operator();
};

inline constexpr auto default_value = [](auto value) static
{
    return function_value_annotation([value = std::move(value)]() { return value; });
};

template <typename T>
struct [[=initializer]] function_array_annotation
{
    const T* data;

    size_t size;

    consteval function_array_annotation(std::initializer_list<T> init) : data(define_static_array(init).data()), size(init.size()) { }

    constexpr function_array_annotation(const T* data, size_t size) : data(data), size(size) { }

    // The range should be constructible from random_access_iterator, which is the case for most of the standard containers.
    template <std::ranges::range R>
    constexpr operator R() const { return R(data, data + size); }

    // We assume the value can get the value by invoke itself, so we return itself here
    // and try cast it to the target type in value.
    constexpr auto& operator()() const { return *this; }
};

inline constexpr auto default_array = []<typename T>(std::initializer_list<T> values) static
{
    return function_array_annotation<T>(values);
};

template <typename Prediction>
struct [[=value_guard]] guard : callable<Prediction>
{
    using callable<Prediction>::callable;
    using callable<Prediction>::operator();
};

inline constexpr auto choice = []<typename... Ts>(Ts&&... ts) 
{
    return guard([...ts=(Ts&&)ts](const auto& value) {
        template for (const auto& element : std::make_tuple((Ts&&)ts...))
            if (element == value)
                return true;
        return false;
    });
};

inline constexpr auto range = []<typename Lower, typename Upper>(Lower lower, Upper upper) 
{
    return guard([lower, upper](const auto& value) {
        return value >= lower && value <= upper;
    });
};



} // namespace cpp::refl


namespace cpp::refl
{



} // namespace cpp::refl

