/*
    -----------------------------------------------------
    |                1   2   3   ?                      |
    -----------------------------------------------------
   start           read         write                 finish
*/

#pragma once

#include "common.hpp"

#include <memory>
#include <bit>
#include <compare>

namespace leviathan::collections
{
    template <typename T, typename Allocator = std::allocator<T>>
    class ring_buffer
    {
        using alloc_traits = std::allocator_traits<Allocator>;

    public:

        using value_type = T;
        using pointer = typename alloc_traits::pointer;
        using const_pointer = typename alloc_traits::const_pointer; 
        using reference = value_type&;
        using const_reference = const value_type&;
        using size_type = size_t;
        using difference_type = std::make_signed_t<size_type>;
        using allocator_type = Allocator;

    private:

        struct offset_helper
        {
            size_type m_mask;
            size_type m_index;

            offset_helper& operator++()
            {
                m_index = (m_index + 1) % m_mask; 
                return *this;
            }

            size_t operator*() const
            { return m_index; }
        };

        struct ring_buffer_iterator
        {
            using value_type = T;
            using reference = T&;
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;

            // For simplify, we just reuse index and operator[] instead of pointer.
            ring_buffer* m_buf = nullptr;

            // We use size_t since the idx is started from 0. However, some methods
            // require difference_type which is ssize_t/ptrdiff_t as their parameter.
            // The users should keep it correct by themselves.
            size_t m_idx = 0;

            ring_buffer_iterator() = default;

            ring_buffer_iterator(const ring_buffer_iterator&) = default;

            ring_buffer_iterator(ring_buffer* buf, size_t idx) : m_buf(buf), m_idx(idx) { }

            reference operator*() const
            { return m_buf->operator[](m_idx); }

            auto operator->() const
            { return &(**this); }

            bool operator==(const ring_buffer_iterator&) const = default;

            auto operator<=>(const ring_buffer_iterator& rhs) const
            {
                // We assume m_buf is equal t0 rhs.m_buf. 
                return m_idx <=> rhs.m_idx; 
            }

            ring_buffer_iterator& operator++()
            {
                m_idx++;
                return *this;
            }

            ring_buffer_iterator operator++(int)
            { 
                auto old = *this;
                ++*this;
                return old;
            }

            ring_buffer_iterator& operator--()
            {
                m_idx--;
                return *this;
            }

            ring_buffer_iterator operator--(int)
            { 
                auto old = *this;
                --*this;
                return old;
            }

            ring_buffer_iterator& operator+=(difference_type n) 
            {
                *this = *this + n;
                return *this;
            }

            ring_buffer_iterator operator+(difference_type n) const
            { return { m_buf, m_idx + n }; }

            friend ring_buffer_iterator operator+(difference_type n, const ring_buffer_iterator& rhs) 
            { return rhs + n; }

            ring_buffer_iterator operator-(difference_type n) const
            { return { m_buf, m_idx - n }; }

            difference_type operator-(const ring_buffer_iterator& rhs) const
            { return static_cast<difference_type>(m_idx) - static_cast<difference_type>(rhs.m_idx); }

            ring_buffer_iterator& operator-=(difference_type n) 
            {
                m_idx -= n;
                return *this;
            }

            reference operator[](difference_type n) const
            {
                assert(n >= 0);
                return m_buf->operator[](m_idx + n);
            }
        };

        struct impl 
        {
            T* m_start = nullptr;
            size_type m_read = 0;
            size_type m_write = 0;
            size_type m_size = 0;
            size_type m_capacity = 0;

            T* read_ptr() 
            { return m_start + m_read; }

            const T* read_ptr() const
            { return m_start + m_read; }

            T* write_ptr()
            { return m_start + m_write; }

            const T* write_ptr() const
            { return m_start + m_write; }

            void forward_read_ptr()
            { m_read = (m_read + 1) % m_capacity; }

            void backward_read_ptr()
            {
                if (m_read == 0)
                {
                    m_read = m_capacity - 1;
                }
                else
                {
                    m_read--;
                }
            }

            void forward_write_ptr()
            { m_write = (m_write + 1) % m_capacity; }

            void backward_write_ptr()
            {
                if (m_write == 0)
                {
                    m_write = m_capacity - 1;
                }
                else
                {
                    m_write--;
                }
            }

            void initialize(Allocator& alloc, size_t capacity)
            {
                m_start = alloc_traits::allocate(alloc, capacity);
                m_capacity = capacity;
                m_size = m_read = m_write = 0;
            }

            void clear(Allocator& alloc)
            {
                offset_helper oh { .m_mask = m_capacity, .m_index = m_read };

                for (size_type i = 0; i < m_size; ++i, ++oh)
                {
                    alloc_traits::destroy(alloc, m_start + *oh);
                }

                m_read = m_write = m_size = 0;
            }

            void dispose(Allocator& alloc)
            {
                clear(alloc);
                alloc_traits::deallocate(alloc, m_start, m_capacity);
                m_start = nullptr;
                m_capacity = 0;
            }
        
            bool is_contiguous() const
            { return m_read + m_size <= m_capacity; }

            bool operator==(const impl&) const = default;
        };

        [[no_unique_address]] Allocator m_alloc;
        impl m_impl;

        // Create a new buffer and move current elements to it.
        // After expanding, the elements are in [start/read, write).
        void expand_capacity_unchecked(size_type capacity)
        {
            if (empty())
            {
                // Just adjust the capacity.
                alloc_traits::deallocate(m_alloc, m_impl.m_start, m_impl.m_capacity);
                m_impl.initialize(m_alloc, capacity);
                return;
            }

            impl store;
            store.initialize(m_alloc, capacity);

            auto deleter = [&](impl* buf) {
                buf->dispose(m_alloc);
            };

            std::unique_ptr<impl, decltype(deleter)> guard(&store, deleter);

            // There are two situations:
            // 1. The write is the right side.
            // 2. The read is the right side.

            if (m_impl.m_write > m_impl.m_read)
            {
                // The elements are in [read, write).
                for (size_type i = m_impl.m_read; i != m_impl.m_write; ++i, ++store.m_write, ++store.m_size)
                {
                    alloc_traits::construct(m_alloc, store.m_start + store.m_write, std::move_if_noexcept(*(m_impl.m_start + i)));
                }
            }
            else
            {
                // The elements are in [read, finish) and [start, write).
                for (size_type i = m_impl.m_read; i != m_impl.m_capacity; ++i, store.m_write++, ++store.m_size)
                {
                    alloc_traits::construct(m_alloc, store.m_start + store.m_write, std::move_if_noexcept(*(m_impl.m_start + i)));
                }
                for (size_type i = 0; i != m_impl.m_write; ++i, store.m_write++, ++store.m_size)
                {
                    alloc_traits::construct(m_alloc, store.m_start + store.m_write, std::move_if_noexcept(*(m_impl.m_start + i)));
                }
            }

            // See buffer.hpp
            std::swap(store, m_impl);
        }

        void try_expand()
        {
            if (size() == capacity())
            {
                expand_capacity_unchecked(std::bit_ceil(size() + 1));
            }
        }

        template <typename... Args>
        reference emplace_back_unchecked(Args&&... args)
        {
            alloc_traits::construct(m_alloc, m_impl.m_start + m_impl.m_write, (Args&&) args...);
            m_impl.forward_write_ptr();
            m_impl.m_size++;
            return back();
        }

        template <typename... Args>
        reference emplace_front_unchecked(Args&&... args)
        {
            m_impl.backward_read_ptr();
            alloc_traits::construct(m_alloc, m_impl.m_start + m_impl.m_read, (Args&&) args...);
            m_impl.m_size++;
            return front();
        }

    public:

        using iterator = ring_buffer_iterator;             
        using const_iterator = std::const_iterator<iterator>;  
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        void show() const
        {
            auto cur = m_impl;
            size_t step = 0;
            do {
                std::cout << *cur.read_ptr() << '\n';
                cur.forward_read_ptr();
            } while (++step != size());
        }

        ring_buffer() = default;

        // TODO:
        ring_buffer(const ring_buffer&) = delete;

        ring_buffer(ring_buffer&&) = delete;

        explicit ring_buffer(const Allocator& allocator) : m_alloc(allocator) 
        { }

        ~ring_buffer()
        { m_impl.dispose(m_alloc); }

        iterator begin()
        { return { this, 0 }; }

        const_iterator begin() const
        { return const_cast<ring_buffer&>(*this).begin(); }

        const_iterator cbegin() const
        { return begin(); }

        reverse_iterator rbegin()
        { return end(); }

        const_reverse_iterator rbegin() const
        { return end(); }

        const_reverse_iterator rcbegin() const
        { return end(); }

        iterator end()
        { return { this, size() }; }

        const_iterator end() const
        { return const_cast<ring_buffer&>(*this).end(); }

        const_iterator cend() const
        { return end(); }

        reverse_iterator rend()
        { return begin(); }

        const_reverse_iterator rend() const
        { return begin(); }

        const_reverse_iterator rcend() const
        { return begin(); }

        void reserve(size_type capacity)
        { expand_capacity_unchecked(capacity); }

        size_type capacity() const 
        { return m_impl.m_capacity; }

        size_type size() const
        { return m_impl.m_size; }
        
        bool empty() const
        { return size() == 0; }

        allocator_type get_allocator() const
        { return m_alloc; }

        void push_back(const T& value)
        { emplace_back(value); }

        void push_back(T&& value)
        { emplace_back(std::move(value)); }

        template <typename... Args>
        reference emplace_back(Args&&... args)
        {
            if (size() == capacity())
            {
                value_handle<value_type, allocator_type> handle(m_alloc, (Args&&) args...);
                expand_capacity_unchecked(std::bit_ceil(size() + 1));
                return emplace_back_unchecked(*handle);
            }
            return emplace_back_unchecked((Args&&) args...);
        }

        void pop_back()
        {
            m_impl.backward_write_ptr();
            alloc_traits::destroy(m_alloc, m_impl.write_ptr());
            m_impl.m_size--;
        }

        void push_front(const T& value)
        { emplace_back(value); }

        void push_front(T&& value)
        { emplace_back(std::move(value)); }

        template <typename... Args>
        reference emplace_front(Args&&... args)
        {
            if (size() == capacity())
            {
                value_handle<value_type, allocator_type> handle(m_alloc, (Args&&) args...);
                expand_capacity_unchecked(std::bit_ceil(size() + 1));
                return emplace_front_unchecked(*handle);
            }
            return emplace_front_unchecked((Args&&) args...);
        }

        void pop_front()
        {
            alloc_traits::destroy(m_alloc, m_impl.read_ptr());
            m_impl.forward_read_ptr();
            m_impl.m_size--;
        }

        T& back()
        {
            auto dest = m_impl.m_write;
            return dest ? *(m_impl.write_ptr() - 1) : *(m_impl.m_start + m_impl.m_capacity - 1);
        }

        const T& back() const
        { return const_cast<ring_buffer&>(*this).back(); }

        T& front()
        { return *m_impl.read_ptr(); }

        const T& front() const
        { return const_cast<ring_buffer&>(*this).front(); }

        void clear()
        { m_impl.clear(m_alloc); }

        void shrink_to_fit()
        { expand_capacity_unchecked(size()); }

        T& operator[](size_type idx)
        { 
            assert(idx < size());
            if (m_impl.m_write > m_impl.m_read)
            {
                return *(m_impl.read_ptr() + idx);
            }
            else
            {
                auto offset = m_impl.m_read + idx;
                return offset >= capacity() ? m_impl.m_start[offset % capacity()] : *(m_impl.read_ptr() + idx);
            }
        }

        const T& operator[](size_type idx) const    
        { return const_cast<ring_buffer&>(*this).operator[](idx); }

        bool is_contiguous() const
        { return m_impl.is_contiguous(); }

    };


} // namespace leviathan::collections

