#pragma once

#include <memory>

#include <assert.h>

namespace leviathan::collections
{
    // Difference from std::vector, this class does not contain allocator
    template <typename T, typename Allocator>
    struct buffer
    {
        T* m_start = nullptr;
        T* m_finish = nullptr;
        T* m_end_of_storage = nullptr;

        buffer() = default;

        buffer(const buffer&) = delete;

        buffer(Allocator& allocator, size_t size)
        {
            m_start = (T*)std::allocator_traits<Allocator>::allocate(allocator, size);
            m_finish = m_start;
            m_end_of_storage = m_start + size;
        }

        constexpr T* begin()
        {
            return m_start;
        }

        constexpr const T* begin() const
        {
            return m_start;
        }

        constexpr T* end()
        {
            return m_finish;
        }

        constexpr const T* end() const
        {
            return m_finish;
        }

        constexpr size_t size() const
        {
            return m_finish - m_start;
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

        // ~buffer()
        constexpr void dispose(Allocator& allocator)
        {
            if (m_start)
            {
                clear(allocator);
                std::allocator_traits<Allocator>::deallocate(allocator, begin(), capacity());
                m_start = m_finish = m_end_of_storage = nullptr;
            }
        }

        constexpr void clear(Allocator& allocator)
        {
            for (; m_finish != m_start;)
            {
                std::allocator_traits<Allocator>::destroy(allocator, --m_finish);
            }
        }

        constexpr void reserve(Allocator& allocator, size_t n)
        {
            try_expand(allocator, n);
        }

        template <typename... Args>
        constexpr T* emplace_back(Allocator& allocator, Args&&... args)
        {
            if (m_finish == m_end_of_storage)
                expand_unchecked_capacity(allocator, std::bit_ceil(size() + 1));

            std::allocator_traits<Allocator>::construct(allocator, m_finish, (Args&&)args...);
            // If an exceptions is thrown above, the m_finish will not increase.
            m_finish++;
            return m_finish;
        }

        template <typename... Args>
        T* emplace(Allocator& allocator, const T* position, Args&&... args) 
        {
            assert(m_start <= position && position <= m_finish && "invalid position");
            
            // We store distance since try_expand may cause memory reallocate 
            // which may make position invalid.
            const auto dist = position - m_start; 

            // try_expand(allocator, size() + 1);
            if (m_finish == m_end_of_storage)
                expand_unchecked_capacity(allocator, std::bit_ceil(size() + 1));

            if (dist == size())
            {
                return emplace_back(allocator, (Args&&) args...);
            }
            else
            {
                T* dest = m_start + dist;

                // What if an exception is thrown when moving?
                std::allocator_traits<Allocator>::construct(allocator, m_finish, std::move(*(m_finish - 1)));
                std::move_backward(dest, m_finish - 1, m_finish);
                std::allocator_traits<Allocator>::destroy(allocator, dest);
                std::allocator_traits<Allocator>::construct(allocator, dest, (Args&&) args...);
                ++m_finish;
                return dest;
            }
        }

        constexpr void pop_back(Allocator& allocator)
        {
            assert(size() > 0 && "Empty buffer.");
            std::allocator_traits<Allocator>::destroy(allocator, --m_finish);
        }

        constexpr void swap(buffer& other)
        {
            std::swap(m_start, other.m_start);
            std::swap(m_finish, other.m_finish);
            std::swap(m_end_of_storage, other.m_end_of_storage);
        }

        void expand_unchecked_capacity(Allocator& allocator, size_t n)
        {
            assert(std::popcount(n) == 1);

            buffer new_buffer(allocator, n);

            // There are two situations:
            // 1. T models nothrow constructible.
            // 2. T does not model nothrow constructible.
            // We simply use a helper class to recover state if exception is thrown. 

            // struct state_guard
            // {
            //     buffer* m_b;
            //     Allocator* m_a;

            //     ~state_guard()
            //     {
            //         m_b->dispose(*m_a);
            //     }
            // };

            // state_guard guard(&new_buffer, &allocator);

            auto deleter = [&](buffer* b) {
                b->dispose(allocator);
            };

            std::unique_ptr<buffer, decltype(deleter)> guard(&new_buffer, deleter);

            auto& dest = new_buffer.m_finish;

            for (auto ptr = m_start; ptr != m_finish; ++ptr)
            {
                std::allocator_traits<Allocator>::construct(allocator, dest, std::move_if_noexcept(*ptr));
                dest++;
            }

            // If all elements are moved/copied into new_buffer, we swap buffer and
            // dispose the current buffer.
            // If any exception is thrown when copying, follow swap will not invoke and
            // guard will dispose the new buffer.
            new_buffer.swap(*this);
        }

        void try_expand(Allocator& allocator, size_t n)
        {
            if (capacity() < n)
                expand_unchecked_capacity(allocator, std::bit_ceil(n));
        }
    };
} // namespace leviathan

