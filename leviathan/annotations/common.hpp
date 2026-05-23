#pragma once

/*
    https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3394r4.html

    How can we parse a json-string to some structure?

    Firstly, we need to parse the json-string to some intermediate representation, 
    which can be a map of string to string, or a vector of string, etc. 
    Then we can use the intermediate representation to construct the final structure.

    For constructing the final structure, we can use some annotations to specify how 
    to map the intermediate representation to the final structure. In that case, we
    must be able to get the annotation of a field, and then use the annotation to get the
    mapping function, and then use the mapping function to get the final value of the field.

    So we need some annotations to specify the mapping function for each field. 
    For example, we can have a shortname annotation, which specifies that the field should 
    be mapped to a short name, and a longname annotation, which specifies that the 
    field should be mapped to a long name, etc.

    Follows are our annotation:

    - rename: specify the name of the field.
    
    - skip: format or scan maybe ignored some fields, this annotation is used to mark those fields.
    
    - value: specify the default value of the field, this is useful when the field is missing.
    
    - parser: for given type T and input type I, call std::invoke(parser, instanceof I) and cast result to T. 

    - required: mark a field as required, if the field is missing, throw an error.

    TODO:
    
    - choice: for given type T and a list of options, try to cast input to each option, 
    if any option is successful, return the result, otherwise throw an error.

*/

#pragma once

#include <string>
#include <functional>
#include <ranges>
#include <algorithm>
#include <meta>

namespace cpp::refl
{
inline constexpr struct { } choice_annotation;

inline constexpr struct { } skip;

inline constexpr struct { } skip_serialization;

inline constexpr struct { } skip_deserialization;

inline constexpr struct { } rename_annotation;

inline constexpr struct { } value_annotation;

/**
 * @brief Check if the given annotation is present on the given info.
 * @param r Anything that can be reflected, such as class, field, base class, etc.
 * @param obj The annotation to check.
 * @return true if the annotation is present, false otherwise.
 * @example 
 * struct SomeThing { [[=some_annotation]] int x; }
 * static_assert(has_annotation(^^SomeThing::x, some_annotation));
 */
template <typename... Ts>
consteval bool has_annotation(std::meta::info r, const Ts&... objs) 
{
    return (... || std::ranges::contains(
        annotations_of_with_type(r, ^^Ts),
        std::meta::reflect_constant(objs),
        std::meta::constant_of
    ));
}

// struct annotation { }; 

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

}  // namespace cpp::refl

