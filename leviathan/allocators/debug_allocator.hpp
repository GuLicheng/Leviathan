#pragma once

#include <memory>
#include <vector>
#include <format>
#include <meta>

namespace cpp::alloc
{

inline static size_t counter = 0;  
inline static std::vector<std::string> messages;

template <typename T>
struct debug_allocator : std::allocator<T>
{
    constexpr T* allocate(size_t n)
    {
        counter += sizeof(T) * n;
        messages.emplace_back(std::format("Allocating {}: {} bytes", display_string_of(^^T), sizeof(T) * n));
        return std::allocator<T>::allocate(n);
    }

    constexpr void deallocate(T* p, size_t n)
    {
        counter -= sizeof(T) * n;
        messages.emplace_back(std::format("Deallocating {}: {} bytes", display_string_of(^^T), sizeof(T) * n));
        return std::allocator<T>::deallocate(p, n);
    }
};

} // namespace cpp::alloc

