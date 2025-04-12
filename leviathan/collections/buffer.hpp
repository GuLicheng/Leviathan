#pragma once

#include "common.hpp"

#include <memory>
#include <algorithm>
#include <initializer_list>
#include <assert.h>

namespace cpp::collections
{
// Difference from std::vector, this class does not contain allocator.
// Any container can split two parts: implementation and allocator. Some containers 
// such as hashtable with chain(std::unordered_map) require dynamic array as 
// for their implementation. If we use std::vector, the allocator in std::vector
// will cost more memory and for user defined allocator, the code may be more complex.
// All APIs are similar to std::vector but the first argument is Allocator&.
template <typename T>
struct buffer
{
    // We use raw pointer, so please make sure your allocator::pointer is not a fancy pointer.
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;

    using iterator = pointer;
    using const_iterator = const_pointer;

    template <typename Allocator>
    using rebind_traits = typename std::allocator_traits<Allocator>::template rebind_traits<T>;

    pointer m_start = nullptr;
    pointer m_finish = nullptr;
    pointer m_end_of_storage = nullptr;

    constexpr buffer() = default;

    constexpr buffer(const buffer&) = default; 

    template <typename Allocator>
    constexpr buffer(Allocator& allocator, std::initializer_list<T> il) 
        : buffer(allocator, il.size())
    {
        auto alloc = detail::rebind_allocator<T>(allocator);
        for (const auto& value : il)
            rebind_traits<Allocator>::construct(alloc, m_finish++, value);
    }

    template <typename Allocator>
    constexpr buffer(Allocator& allocator, size_t size)
    {
        auto alloc = detail::rebind_allocator<T>(allocator);
        const auto sz = std::bit_ceil(size);
        m_start = rebind_traits<Allocator>::allocate(alloc, sz);
        m_finish = m_start;
        m_end_of_storage = m_start + sz;
    }

    constexpr iterator begin()
    {
        return m_start;
    }

    constexpr iterator end()
    {
        return m_finish;
    }

    constexpr const_iterator begin() const
    {
        return m_start;
    }

    constexpr const_iterator end() const
    {
        return m_finish;
    }

    constexpr const_iterator cbegin() const
    {
        return m_start;
    }

    constexpr const_iterator cend() const
    {
        return m_finish;
    }

    constexpr size_t size() const
    {
        return m_finish - m_start;
    }

    constexpr ssize_t ssize() const
    {
        return static_cast<ssize_t>(size());
    }

    constexpr size_t capacity() const
    {
        return m_end_of_storage - m_start;
    }

    constexpr size_t empty() const
    {
        return size() == 0;
    }

    constexpr T& operator[](size_t n)
    {
        assert(n < size() && "invalid index");
        return m_start[n];
    }

    constexpr const T& operator[](size_t n) const
    {
        assert(n < size() && "invalid index");
        return m_start[n];
    }

    constexpr const T& front() const
    {
        assert(!empty() && "buffer has no element!");
        return *m_start;
    }

    constexpr T& front() 
    {
        assert(!empty() && "buffer has no element!");
        return *m_start;
    }

    constexpr const T& back() const
    {
        assert(!empty() && "buffer has no element!");
        return *(m_finish - 1);
    }

    constexpr T& back() 
    {
        assert(!empty() && "buffer has no element!");
        return *(m_finish - 1);
    }

    // ~buffer()
    template <typename Allocator>
    constexpr void dispose(Allocator& allocator)
    {
        if (m_start)
        {
            auto alloc = detail::rebind_allocator<T>(allocator);
            clear(alloc);
            rebind_traits<Allocator>::deallocate(alloc, begin(), capacity());
            m_start = m_finish = m_end_of_storage = nullptr;
        }
    }

    template <typename Allocator>
    constexpr void clear(Allocator& allocator)
    {
        auto alloc = detail::rebind_allocator<T>(allocator);
        for (; m_finish != m_start;)
        {
            rebind_traits<Allocator>::destroy(alloc, --m_finish);
        }
    }

    template <typename Allocator>
    constexpr void reserve(Allocator& allocator, size_t n)
    {
        try_expand(allocator, n);
    }

    template <typename Allocator, typename... Args>
    constexpr iterator emplace_back(Allocator& allocator, Args&&... args)
    {
        auto alloc = detail::rebind_allocator<T>(allocator);

        if (m_finish == m_end_of_storage)
        {
            // See buffer::emplace.
            value_handle<value_type, decltype(alloc)> handle(alloc, (Args&&)args...);
            expand_capacity_unchecked(alloc, std::bit_ceil(size() + 1));
            return emplace_back_unchecked(alloc, *handle);
        }

        return emplace_back_unchecked(alloc, (Args&&)args...);
    }

    template <typename Allocator, typename... Args>
    constexpr iterator emplace_back_unchecked(Allocator& allocator, Args&&... args)
    {
        auto alloc = detail::rebind_allocator<T>(allocator);
        rebind_traits<Allocator>::construct(alloc, m_finish, (Args&&)args...);
        // If an exceptions is thrown above, the m_finish will not increase.
        m_finish++;
        return m_finish;
    }

    // template <typename Range>
    // constexpr iterator insert_range(Allocator& allocator, const_iterator pos, Range&& r)
    // {
    //     return insert(allocator, pos, std::ranges::begin(r), std::ranges::end(r));
    // }

    template <typename Allocator, typename InputIterator>
    constexpr iterator insert(Allocator& allocator, const_iterator pos, InputIterator first, InputIterator last)
    {
        if (first == last)
        {
            return const_cast<iterator>(pos);
        }
        auto alloc = detail::rebind_allocator<T>(allocator);
        assert(m_start <= pos && pos <= m_finish && "Invalid position");

        const auto d1 = pos - cbegin();
        const auto d2 = size();

        // Reserve enough memory if possible
        if constexpr (std::random_access_iterator<InputIterator>)
        {
            const auto dist = std::distance(first, last);
            try_expand(alloc, dist + size());
            for (; first != last; ++first)
            {
                rebind_traits<Allocator>::construct(alloc, m_finish++, *first);
            }
        }
        else
        {
            for (; first != last; ++first)
            {
                emplace_back(alloc, *first);
            }
        }

        // It's difficult to move elements first since the memory of [m_finish, m_end_of_capacity)
        // is uninitialized. So we emplace them at the end and rotate them to correct 
        // position. This technique can be seen in some STL's implementation. 
        std::rotate(begin() + d1, begin() + d2, end());
        return begin() + d1;
    }

    template <typename Allocator>
    constexpr iterator erase(Allocator& allocator, const_iterator pos)
    {
        assert(m_start <= pos && pos < m_finish && "Invalid position");
        auto alloc = detail::rebind_allocator<T>(allocator);

        iterator dest = const_cast<iterator>(pos);
        std::move(dest + 1, m_finish, dest);
        // Remove last element
        rebind_traits<Allocator>::destroy(alloc, --m_finish);
        return dest;
    }

    template <typename Allocator>
    constexpr iterator erase(Allocator& allocator, const_iterator first, const_iterator last)
    {
        if (first == last)
        {
            return const_cast<iterator>(first);
        }

        assert(m_start <= first && first < last && "Invalid position");
        assert(m_start < last && last <= last && "Invalid position");
        auto alloc = detail::rebind_allocator<T>(allocator);

        if (last == cend())
        {
            iterator dest = const_cast<iterator>(first);
            for (; m_finish != dest;)
                rebind_traits<Allocator>::destroy(alloc, --m_finish);
            return dest;
        }

        // Move [last, m_finish) to [first, first + (last - first)) and 
        // erase [first + (last - first), m_finish)
        std::move(const_cast<iterator>(last), end(), const_cast<iterator>(first));
        iterator dest = const_cast<iterator>(first + (cend() - last));
        for (; m_finish != dest;)
        {
            rebind_traits<Allocator>::destroy(alloc, --m_finish);
        }

        return const_cast<iterator>(first);
    }

    template <typename Allocator, typename... Args>
    constexpr iterator emplace(Allocator& allocator, const_pointer position, Args&&... args) 
    {
        assert(m_start <= position && position <= m_finish && "invalid position");
        auto alloc = detail::rebind_allocator<T>(allocator);

        // Something in args... could alias one of the elements of the container.
        // For instance: 
        //      buffer.insert(allocator, buffer.begin(), buffer[0]);
        // We cannot access the correct buffer[0] since 
        // the buffer[0] is already moved to buffer[1]. So
        // we store the value and then move to the position.
        value_handle<T, Allocator> handle(alloc, (Args&&) args...);

        // We store distance since try_expand may cause memory reallocate 
        // which may make position invalid.
        const size_t dist = position - m_start; 

        // try_expand(allocator, size() + 1);
        if (m_finish == m_end_of_storage)
        {
            expand_capacity_unchecked(alloc, std::bit_ceil(size() + 1));
        }

        if (dist == size())
        {
            // return emplace_back(alloc, (Args&&) args...);
            return emplace_back_unchecked(alloc, (Args&&) args...);
        }
        else
        {
            auto dest = m_start + dist;

            // What if an exception is thrown when moving?
            rebind_traits<Allocator>::construct(alloc, m_finish, std::move(*(m_finish - 1)));
            std::move_backward(dest, m_finish - 1, m_finish);
            rebind_traits<Allocator>::destroy(alloc, dest);
            rebind_traits<Allocator>::construct(alloc, dest, *handle);
            ++m_finish;
            return dest;
        }
    }

    template <typename Allocator>
    constexpr void pop_back(Allocator& allocator)
    {
        assert(size() > 0 && "Empty buffer.");
        auto alloc = detail::rebind_allocator<T>(allocator);
        rebind_traits<Allocator>::destroy(alloc, --m_finish);
    }

    constexpr void swap(buffer& other) noexcept
    {
        using std::swap;
        swap(m_start, other.m_start);
        swap(m_finish, other.m_finish);
        swap(m_end_of_storage, other.m_end_of_storage);
    }

    constexpr friend void swap(buffer& lhs, buffer& rhs) noexcept
    {
        lhs.swap(rhs);
    }

    template <typename Allocator>
    constexpr void expand_capacity_unchecked(Allocator& allocator, size_t n)
    {
        assert(std::popcount(n) == 1);
        auto alloc = detail::rebind_allocator<T>(allocator);

        buffer new_buffer(alloc, n);

        // There are two situations:
        // 1. T models nothrow constructible.
        // 2. T does not model nothrow constructible.
        // We simply use a helper class to recover state if exception is thrown. 

        auto deleter = [&](buffer* b) {
            b->dispose(alloc);
        };

        std::unique_ptr<buffer, decltype(deleter)> guard(&new_buffer, deleter);

        auto& dest = new_buffer.m_finish;

        for (auto ptr = m_start; ptr != m_finish; ++ptr)
        {
            rebind_traits<Allocator>::construct(alloc, dest, std::move_if_noexcept(*ptr));
            dest++;
        }

        // If all elements are moved/copied into new_buffer successfully, we swap buffers and
        // dispose the current buffer.
        // If any exception is thrown when copying, following swap will not be invoked and
        // the guard will dispose the new buffer.
        new_buffer.swap(*this);
    }

    template <typename Allocator>
    constexpr void try_expand(Allocator& allocator, size_t n)
    {
        if (capacity() < n)
        {
            auto alloc = detail::rebind_allocator<T>(allocator);
            expand_capacity_unchecked(alloc, std::bit_ceil(n));
        }
    }

 };

} // namespace cpp

