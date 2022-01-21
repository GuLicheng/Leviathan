#pragma once

#include <vector> // bit map storage
#include <iostream>
#include <assert.h>

namespace leviathan::collections
{
template <int BitWidth, int BitCount = 10>
class bitmap
{
    static_assert((unsigned)BitWidth < 8, "BitWidth must be less than 8. And use uint8_t[] instead");
    constexpr static int overflow = 1 << BitWidth;
    std::vector<bool> m_store;

    template <bool Const>
    struct proxy
    {
        bitmap* m_ptr;
        size_t m_idx;

        proxy(bitmap* ptr, size_t idx) : m_ptr{ ptr }, m_idx{ idx } { } 
        proxy(const proxy&) = delete;
        proxy& operator=(const proxy&) = delete;

        proxy& operator=(size_t index) && requires(!Const)
        {
            assert(index < overflow);
            size_t start = m_idx * BitWidth;
            for (size_t i = start; index; ++i, index >>= 1) 
            {
                m_ptr->m_store[i] = index & 1;
            }
            return *this;
        }
    
        operator int() &&
        {
            size_t offset = m_idx * BitWidth;
            int val = 0;
            // for (size_t i = 0; i < BitWidth; ++i) 
            for (size_t i = BitWidth - 1; i != static_cast<size_t>(-1); --i) 
            {
                val <<= 1;
                bool bit = m_ptr->m_store[i + offset];
                val |= bit;
                // std::cout << "i = " << i << " and value = " << static_cast<int>(m_ptr->m_store[i]) << '\n';
            }
            return val;
        }

    };

public:

    bitmap() 
    {
        m_store.resize(BitCount);
    }

    proxy<false> operator[](size_t index) 
    {
        return { this, index }; 
    }

    proxy<true> operator[](size_t index) const
    {
        return { this, index }; 
    }

    void display() const 
    {
        for (auto i : m_store)
        {
            std::cout << i << ' ';
        }
        std::cout << '\n';
    }

};

} // namespace leviathan::collections

