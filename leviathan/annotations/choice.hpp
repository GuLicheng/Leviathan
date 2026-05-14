#pragma once

#include <leviathan/annotations/common.hpp>

namespace cpp::refl
{

inline constexpr struct { } choice_annotation;

template <typename Prediction>
struct [[=choice_annotation]] function_choice_annotation : callable<Prediction>
{
    using callable<Prediction>::callable;
    using callable<Prediction>::operator();
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
