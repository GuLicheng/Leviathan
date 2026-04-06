/*
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

*/

#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <vector>
#include <ranges>
#include <algorithm>

namespace cpp
{

// ------------------ annotation ------------------

struct annotation { }; 

struct rename_annotation : annotation { };

struct debug_annotation : annotation { };

template <typename T>
struct value_annotation : annotation 
{
    T value;

    constexpr explicit value_annotation(T value) : value(std::move(value)) {}
};
    
struct help_annotation : value_annotation<std::string_view> 
{
    using value_annotation::value_annotation;
};

// ------------------ rename annotations ------------------
template <typename F>
struct function_rename_annotation : rename_annotation
{
    F function;

    constexpr explicit function_rename_annotation(F function) : function(std::move(function)) {}

    template <typename Self>
    constexpr std::string operator()(this Self&& self, std::string_view field_name) 
    {
        return std::invoke(((Self&&)self).function, field_name);
    }
};

struct rename : rename_annotation
{
    std::string_view name;

    constexpr explicit rename(std::string_view name) : name(name) {}

    constexpr std::string operator()(std::string_view field_name) const
    {
        return std::string(name);
    }
};


// ------------------ predefined annotations ------------------

inline constexpr auto help = [](std::string_view message) static
{
    return help_annotation(message);
};

inline constexpr auto shortname = function_rename_annotation([](std::string_view field_name) static 
{
    // assert(!name.empty(), "Name cannot be empty");
    return '-' + std::string(field_name.begin(), field_name.begin() + 1);
});

inline constexpr auto longname = function_rename_annotation([](std::string_view field_name) static 
{
    // assert(!name.empty(), "Name cannot be empty");
    return "--" + std::string(field_name);
});

inline constexpr auto lowercase = function_rename_annotation([](std::string_view field_name) static 
{
    return field_name | std::views::transform(::tolower) | std::ranges::to<std::string>();
});

inline constexpr auto uppercase = function_rename_annotation([](std::string_view field_name) static 
{
    return field_name | std::views::transform(::toupper) | std::ranges::to<std::string>();
});

// struct default_serializer 
// {
//     template <typename T>
//     static constexpr std::string operator()(const T& value);
// };

// struct default_deserializer 
// {
//     template <typename T>
//     static constexpr std::string operator()(const T& value);  
// };

// template <typename Serializer = default_serializer, typename Deserializer = default_deserializer>
// struct serialize_annotation : annotation 
// {
//     Serializer serializer;
//     Deserializer deserializer;

//     constexpr serialize_annotation(Serializer serializer = {}, Deserializer deserializer = {}) 
//         : serializer(std::move(serializer)), deserializer(std::move(deserializer)) {}
// };


}
