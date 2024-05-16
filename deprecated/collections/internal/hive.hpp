#pragma once

#include <initializer_list>
#include <memory>
#include <compare>

namespace leviathan::collections
{
    struct hive_limits
    {
        size_t m_min;
        size_t m_max;
        constexpr hive_limits(size_t minium, size_t maximum) 
            : m_min(minium), m_max(maximum) 
        {
        } 
    };

    template <typename T, typename Allocator = std::allocator<T>> 
    class hive
    {
        struct group
        {
            
        }; 
    };


    template <typename T, typename Allocator>
    void swap(hive<T, Allocator>& x, hive<T, Allocator>& y);



} // namespace leviathan::collections

