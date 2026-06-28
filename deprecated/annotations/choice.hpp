#pragma once

#include <leviathan/annotations/common.hpp>

namespace cpp::refl
{

template <typename Prediction>
struct [[=value_guard]] function_choice_annotation : callable<Prediction>
{
    using callable<Prediction>::callable;
    using callable<Prediction>::operator();
};

inline constexpr auto choice = []<typename... Ts>(Ts&&... ts) 
{
    return function_choice_annotation([...ts=(Ts&&)ts](const auto& value) {
        template for (const auto& element : std::make_tuple((Ts&&)ts...))
            if (element == value)
                return true;
        return false;
    });
};

inline constexpr auto range = []<typename Lower, typename Upper>(Lower lower, Upper upper) 
{
    return function_choice_annotation([lower, upper](const auto& value) {
        return value >= lower && value <= upper;
    });
};

}  // namespace cpp::refl