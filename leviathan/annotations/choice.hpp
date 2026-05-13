#pragma once

#include <leviathan/annotations/common.hpp>

namespace cpp::refl
{

struct choice_annotation : annotation { };

template <typename F>
struct function_choice_annotation : choice_annotation
{
    F function;

    constexpr explicit function_choice_annotation(F function) : function(std::move(function)) {}

    template <typename Self, typename... Args>
    constexpr bool operator()(this Self&& self, Args&&... args) 
    {
        return std::invoke(((Self&&)self).function, (Args&&)args...);
    }
};

inline constexpr auto choice = []<typename... Ts>(Ts&&... ts) 
{
    // Copy
    return function_choice_annotation([...ts=(Ts&&)ts](const auto& value) {

        template for (const auto& element : std::make_tuple((Ts&&)ts...))
        {
            if (element == value)
            {
                return true;
            }
        }
        return false;
    });
};

}
