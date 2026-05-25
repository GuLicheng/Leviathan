
#pragma once

#include <leviathan/annotations/common.hpp>
#include <initializer_list>

namespace cpp::refl
{
    
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

template <typename T>
struct [[=value_annotation]] function_array_annotation
{
    const T* data;

    size_t size;

    consteval function_array_annotation(std::initializer_list<T> init) : data(define_static_array(init).data()), size(init.size()) { }

    constexpr function_array_annotation(const T* data, size_t size) : data(data), size(size) { }

    // The range should be constructible from random_access_iterator, which is the case for most of the standard containers.
    template <std::ranges::range R>
    constexpr operator R() const { return R(data, data + size); }

    // We assume the value_annotation can get the value by invoke itself, so we return itself here
    // and try cast it to the target type in value_annotation.
    constexpr auto& operator()() const { return *this; }
};

inline constexpr auto default_array = []<typename T>(std::initializer_list<T> values) static
{
    return function_array_annotation<T>(values);
};


}  // namespace cpp::refl



