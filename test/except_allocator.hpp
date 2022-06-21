#pragma once

#include <memory>
#include <stdexcept>

constexpr int max_alloc = 5;
int alloc_num = 0;

template <typename T>
class exception_allocator : public std::allocator<T>
{
    constexpr T* allocate(size_t n)
    {
        alloc_num++;
        if (alloc_num >= 5)
            throw std::bad_alloc();
        return std::allocator<T>::allocate(n);
    }
};






