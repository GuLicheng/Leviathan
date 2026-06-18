#pragma once

#include <ranges>
#include <initializer_list>
#include <meta>

namespace cpp
{

template <typename T>
struct array
{
    const T* data;

    size_t size;

    constexpr array(std::initializer_list<T> init) 
        : data(std::define_static_array(init).data()), size(init.size()) {}

    constexpr T& operator[](size_t index) const { return data[index]; }

    constexpr auto begin() const { return data; }

    constexpr auto end() const { return data + size; }

    template <std::ranges::range R>
    constexpr operator R() const
    {
        return R(data, data + size);
    }
};

}  // namespace cpp
