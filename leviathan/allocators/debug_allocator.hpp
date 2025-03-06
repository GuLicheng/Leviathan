#pragma once

#include <memory>
#include <vector>
#include <format>
#include <leviathan/meta/template_info.hpp>

namespace leviathan::alloc
{

inline static size_t counter = 0;  
inline static std::vector<std::string> messages;

template <typename T>
struct debug_allocator : std::allocator<T>
{
    constexpr T* allocate(size_t n)
    {
        counter += sizeof(T) * n;
        messages.emplace_back(std::format("Allocating {}: {} bytes", TypeInfo(T), sizeof(T) * n));
        return std::allocator<T>::allocate(n);
    }

    constexpr void deallocate(T* p, size_t n)
    {
        counter -= sizeof(T) * n;
        messages.emplace_back(std::format("Deallocating {}: {} bytes", TypeInfo(T), sizeof(T) * n));
        return std::allocator<T>::deallocate(p, n);
    }
};

} // namespace leviathan::alloc

