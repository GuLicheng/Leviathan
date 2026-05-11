
#pragma once

#include <leviathan/annotations/common.hpp>

namespace cpp::refl
{

struct parser_annotation : annotation { };


// ------------------- default value annotations -------------------
template <typename T>
struct value_annotation : annotation 
{
    T value;

    constexpr explicit value_annotation(T value) : value(std::move(value)) {}

    constexpr T operator()() const 
    {
        return value;
    }
};
    
inline constexpr auto default_value = [](auto value) static
{
    return value_annotation(std::move(value));
};



template <typename F>
struct function_parser_annotation : parser_annotation
{
    F function;

    constexpr explicit function_parser_annotation(F function) : function(std::move(function)) {}

    template <typename Self, typename... Args>
    constexpr auto operator()(this Self&& self, Args&&... args)
    {
        return std::invoke(((Self&&)self).function, (Args&&)args...);
    }
};

template <std::meta::info Info, typename T>
constexpr auto parser_by_annotation(T value)
{
    template for (constexpr auto anno : define_static_array(annotations_of(Info)))
    {
        using AnnoType = typename [:type_of(anno):];

        if constexpr (std::derived_from<AnnoType, parser_annotation>)
        {
            return extract<AnnoType>(anno)(value);
        }
    }
    throw std::runtime_error("No parser annotation found for " + std::string(identifier_of(Info)));
}


}  // namespace cpp::refl



