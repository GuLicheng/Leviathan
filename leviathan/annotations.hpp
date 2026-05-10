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
#include <string>
#include <functional>
#include <vector>
#include <ranges>
#include <meta>
#include <algorithm>

namespace cpp::refl
{

// ------------------ annotation ------------------

struct annotation { }; 

struct rename_annotation : annotation { };

struct debug_annotation : annotation { };

struct ignore_annotation : annotation { };

template <typename T>
struct value_annotation : annotation 
{
    T value;

    constexpr explicit value_annotation(T value) : value(std::move(value)) {}
};
    
struct help_annotation : value_annotation<const char*> 
{
    using value_annotation::value_annotation;
};

// ------------------ rename annotations ------------------
template <typename F>
struct function_rename_annotation : rename_annotation
{
    F function;

    constexpr explicit function_rename_annotation(F function) : function(std::move(function)) {}

    template <typename Self, typename... Args>
    constexpr std::string operator()(this Self&& self, Args&&... args)
    {
        return std::invoke(((Self&&)self).function, (Args&&)args...);
    }
};

// ------------------ predefined annotations ------------------

inline constexpr auto ignore = ignore_annotation{};

inline constexpr auto help = [](std::string_view message) static
{
    return help_annotation(define_static_string(message));
};

inline constexpr auto shortname = function_rename_annotation([](std::string field_name) static 
{
    // assert(!name.empty(), "Name cannot be empty");
    return '-' + std::string(field_name.begin(), field_name.begin() + 1);
});

inline constexpr auto longname = function_rename_annotation([](std::string field_name) static 
{
    // assert(!name.empty(), "Name cannot be empty");
    return "--" + std::string(field_name);
});

inline constexpr auto lowercase = function_rename_annotation([](std::string field_name) static 
{
    return field_name | std::views::transform(::tolower) | std::ranges::to<std::string>();
});

inline constexpr auto uppercase = function_rename_annotation([](std::string field_name) static 
{
    return field_name | std::views::transform(::toupper) | std::ranges::to<std::string>();
});

inline constexpr auto rename = [](std::string_view new_name) static
{
    return function_rename_annotation([name=define_static_string(new_name)](...)  
    {
        return std::string(name);
    });
};


}  // namespace cpp::refl


// We define some utilities functions
namespace cpp::refl
{


template <std::meta::info Info>
constexpr std::string extract_name_by_annotation()
{
    static_assert(has_identifier(Info), "Info must have an identifier");
 
    auto name = std::string(identifier_of(Info));

    template for (constexpr auto anno : define_static_array(annotations_of(Info)))
    {
        using AnnoType = typename [:type_of(anno):];

        if constexpr (std::is_base_of_v<rename_annotation, AnnoType>)
        {
            name = std::invoke(extract<AnnoType>(anno), name);
        }
    }
    return name;    
}

template <std::meta::info Info>
consteval bool is_ignored()
{
    static_assert(has_identifier(Info), "Info must have an identifier");

    template for (constexpr auto anno : define_static_array(annotations_of(Info)))
    {
        using AnnoType = typename [:type_of(anno):];

        if constexpr (std::is_base_of_v<ignore_annotation, AnnoType>)
        {
            return true;
        }
    }
    return false;
}
    
}  // namespace cpp::refl
