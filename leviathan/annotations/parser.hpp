
#pragma once

#include <leviathan/annotations/common.hpp>

namespace cpp::refl
{

inline constexpr struct { } value_annotation;

template <typename F>
struct [[=value_annotation]] function_value_annotation : callable<F>
{
    using callable<F>::callable;
    using callable<F>::operator();
};

inline constexpr auto default_value = [](auto value) static
{
    return function_value_annotation([value = std::move(value)]() { return value; });
};


}  // namespace cpp::refl



