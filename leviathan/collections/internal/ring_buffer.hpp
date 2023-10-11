/*
                    direction: ----->
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

        constexpr static bool IsNothrowMoveConstruct = 
                    std::is_nothrow_move_constructible_v<T> 
                 && typename alloc_traits::is_always_equal();

        constexpr static bool IsNothrowMoveAssign = 
                    std::is_nothrow_move_assignable_v<T> 
                 && typename alloc_traits::is_always_equal();

        constexpr static bool IsNothrowSwap = 
                    std::is_nothrow_swappable_v<T> 
                 && typename alloc_traits::is_always_equal();

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
                // We assume m_buf and rhs.m_buf are pointing to the same object. 
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

        template <typename Action, typename Impl>
        static void carry_elements(allocator_type& alloc, Impl&& src, impl& dst, Action action) 
        {
            // There are two situations:
            // 1. The write is the right side.
            // 2. The read is the right side.
            if (src.m_write > src.m_read)
            {
                // The elements are in [read, write).
                for (size_type i = src.m_read; i != src.m_write && dst.m_write != dst.m_capacity; ++i, ++dst.m_write, ++dst.m_size)
                {
                    alloc_traits::construct(alloc, dst.m_start + dst.m_write, action(*(src.m_start + i)));
                }
            }
            else
            {
                // The elements are in [read, finish) and [start, write).
                for (size_type i = src.m_read; i != src.m_capacity && dst.m_write != dst.m_capacity; ++i, dst.m_write++, ++dst.m_size)
                {
                    alloc_traits::construct(alloc, dst.m_start + dst.m_write, action(*(src.m_start + i)));
                }
                for (size_type i = 0; i != src.m_write && dst.m_write != dst.m_capacity; ++i, dst.m_write++, ++dst.m_size)
                {
                    alloc_traits::construct(alloc, dst.m_start + dst.m_write, action(*(src.m_start + i)));
                }
            }
        }

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

            carry_elements(m_alloc, m_impl, store, [](auto& x) { return std::move_if_noexcept(x); });

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

        ring_buffer(const ring_buffer& rhs) 
            : ring_buffer(rhs, alloc_traits::select_on_container_copy_construction(rhs.m_alloc)) { }

        ring_buffer(const ring_buffer& rhs, const allocator_type& allocator)
            : m_alloc(allocator)
        {
            try
            {
                m_impl.initialize(m_alloc, rhs.size());
                carry_elements(m_alloc, rhs.m_impl, m_impl, [](const auto& x) -> auto& { return x; });
            }
            catch(...)
            {
                clear();
                throw;
            }
        }

        ring_buffer(ring_buffer&& rhs) noexcept(IsNothrowMoveConstruct)
            : m_impl(std::exchange(rhs.m_impl, impl())), m_alloc(std::move(rhs.m_alloc)) { }

        ring_buffer(ring_buffer&& rhs, const allocator_type& alloc) : m_alloc(alloc)
        {
            if (rhs.m_alloc == alloc)
            {
                m_impl = std::exchange(rhs.m_impl, impl());
            }
            else
            {
                carry_elements(m_alloc, rhs.m_impl, m_impl, [](auto&& x) -> auto&& { return std::move(x); });
                rhs.clear();
            }
        }

        ring_buffer& operator=(const ring_buffer& rhs) 
        {
            if (this != std::addressof(rhs))
            {
                clear();
                if constexpr (typename alloc_traits::propagate_on_container_copy_assignment())
                {
					m_alloc = rhs.m_alloc;
                }
                try
                {
                    m_impl.initialize(m_alloc, rhs.size());
                    carry_elements(m_alloc, rhs.m_impl, m_impl, [](const auto& x) -> auto& { return x; });
                }
                catch(...)
                {
                    clear();
                    throw;
                }
            }
            return *this;
        }

        ring_buffer& operator=(ring_buffer&& rhs) noexcept(IsNothrowMoveAssign)
        {
            if (this != std::addressof(rhs))
            {
                clear();
				if constexpr (typename alloc_traits::propagate_on_container_move_assignment())
				{
					m_alloc = std::move(rhs.m_alloc);
					// Move impl and reset other's state.
                    m_impl = std::exchange(rhs.m_impl, impl());
				}
                else
                {
                    if (m_alloc == rhs.m_alloc)
                    {
                        m_impl = std::exchange(rhs.m_impl, impl());
                    }
                    else
                    {
                        m_impl.initialize(m_alloc, rhs.size());
                        carry_elements(m_alloc, rhs.m_impl, m_impl, [](auto&& x) -> auto&& { return std::move(x); });
                        rhs.clear();
                    }
                }
            }
            return *this;
        }

        explicit ring_buffer(const Allocator& allocator) : m_alloc(allocator) 
        { }

        ~ring_buffer()
        { m_impl.dispose(m_alloc); }

        void swap(ring_buffer& rhs) noexcept(IsNothrowSwap)
        {
            std::swap(m_impl, rhs.m_impl);
            if constexpr (typename alloc_traits::propagate_on_container_swap())
            {
                std::swap(m_alloc, rhs.m_alloc);
            }
        }

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

        void resize(size_type count)
        {
            auto old = size();
            
            if (count == old)
            {
                return;
            }

            expand_capacity_unchecked(count);
            
            if (count > old)
            {
                for (size_type i = old; i != count; ++i)
                {
                    emplace_back_unchecked();
                }
            }
        }

        // void resize(size_type count, const value_type& value);

        template <typename... Args>
        iterator emplace(const_iterator pos, Args&&... args)
        {
            if (size() == capacity())
            {
                expand_capacity_unchecked(std::bit_ceil(size() + 1));
            }

            value_handle<T, allocator_type> handle(m_alloc, (Args&&) args...);

            if (pos == cbegin())
            {
                emplace_front_unchecked(*handle);
                return begin();
            }

            if (pos == cend())
            {
                emplace_back_unchecked(*handle);
                return end() - 1;
            }

            const auto position = pos.base().m_idx;

            if (is_contiguous() && m_impl.m_write != 0)
            {
                auto dest = m_impl.m_start + position;

                // What if an exception is thrown when moving?
                alloc_traits::construct(m_alloc, m_impl.write_ptr(), std::move(*(m_impl.write_ptr() - 1)));
                std::move_backward(dest, m_impl.write_ptr() - 1, m_impl.write_ptr());
                alloc_traits::destroy(m_alloc, dest);
                alloc_traits::construct(m_alloc, dest, *handle);
                m_impl.forward_write_ptr();
                ++m_impl.m_size;
                return { this, position };
            }
            else
            {
                emplace_back_unchecked(*pos);
                std::rotate(begin(), end() - 1, end());
                return { this, position };
            }
        }

        iterator insert(const_iterator pos, const T& value)
        { return emplace(pos, value); }

        iterator insert(const_iterator pos, T&& value)
        { return emplace(pos, std::move(value)); }

        iterator erase(const_iterator first, const_iterator last)
        {
            if (first == cbegin() && last == cend())
            {
                clear();
                return end();
            }

            auto first1 = first.base(), last1 = last.base();
            for (; first1 != last1; first1 = erase(first1));
            return first1;
        }

        iterator erase(const_iterator pos)
        {
            auto idx = pos.base().m_idx;
            assert(idx < size());

            if (is_contiguous())
            {
                // See vector::erase
                auto left = m_impl.read_ptr() + idx;

                // If the ring_buffer is full, the write_ptr will equal to read_ptr, we make
                // right always point the right of read_ptr.
                auto right = m_impl.read_ptr() != m_impl.write_ptr() ? m_impl.write_ptr() : m_impl.m_start + m_impl.m_capacity;
                std::move(left + 1, right, left);
                pop_back();
            }
            else
            {
                auto dest = std::addressof(*pos.base());
                
                if (dest >= m_impl.read_ptr())
                {
                    // Right part
                    std::move_backward(m_impl.read_ptr(), dest, dest + 1);
                    pop_front();
                }
                else
                {
                    // Left part
                    auto right = m_impl.write_ptr();
                    std::move(dest + 1, right, dest);
                    pop_back();
                }
            }
            // If we remove the last element, the idx should be reset to 0.
            return { this, idx == size() ? 0 : idx };
        }
    };
} // namespace leviathan::collections

