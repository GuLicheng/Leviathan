// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0843r2.html
#pragma once

#include <leviathan/allocators/stack_allocator.hpp>

#include <vector>
#include <cstdint>
#include <iterator>
#include <memory>
#include <type_traits>
#include <initializer_list>

#include <assert.h>

namespace cpp::collections
{
/**
 * @brief A variable-size array container with fixed capacity.
 * 
 *  We use std::vector with stack_allocator as our underlying implement. We call
 *  std::vector::reverse(N) in constructor to make sure the static_vector has enough room.
 *  And C++ requires follow operations will not change the std::vector capacity:
 *  - std::vector::resize(n) if n is not greater than capacity
 *  - std::vector::clear 
 * 
 * @param T The type of element that will be stored.
 * @param N The maximum number of elements static_vector can store, fixed at compile time.
*/
template <typename T, size_t N>
class static_vector
{
    using storage = std::vector<T, cpp::alloc::stack_allocator<T, N, true>>;
    storage m_vec;

    void check_overflow(size_t n)
    {
        assert(n <= N);
    }

public:

    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*; 
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = size_t;
    using difference_type = std::make_signed_t<size_type>;
    using iterator = typename storage::iterator;              // This may not suitable.
    using const_iterator = typename storage::const_iterator;  // This may not suitable.
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // 5.2, copy/move construction:
    constexpr static_vector() noexcept 
    { m_vec.reserve(N); }
    
    constexpr explicit static_vector(size_type n)
    {
        check_overflow(n);
        m_vec.reserve(N);
        m_vec.resize(n);
    }
    
    constexpr static_vector(size_type n, const value_type& value)
    {
        check_overflow(n);
        m_vec.reserve(N);
        std::fill_n(std::back_inserter(m_vec), n, value);
    }

    template <std::input_iterator InputIterator>
    constexpr static_vector(InputIterator first, InputIterator last)
    {
        m_vec.reserve(N);
        std::copy(first, last, std::back_inserter(m_vec));
    }

    constexpr static_vector(const static_vector& other)
    noexcept(std::is_nothrow_copy_constructible_v<value_type>) 
        : static_vector(other.begin(), other.end()) 
    { }

    constexpr static_vector(static_vector&& other)
    noexcept(std::is_nothrow_move_constructible_v<value_type>) 
        : static_vector(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()))
    { }

    constexpr static_vector(std::initializer_list<value_type> il)
        : static_vector(il.begin(), il.end()) 
    { }

    // 5.3, copy/move assignment:
    constexpr static_vector& operator=(const static_vector& other)
    noexcept(std::is_nothrow_copy_assignable_v<value_type>)
    {
        if (this != std::addressof(other))
        {
            assign(other.begin(), other.end());
        }
        return *this;
    }

    constexpr static_vector& operator=(static_vector&& other)
    noexcept(std::is_nothrow_move_assignable_v<value_type>)
    {
        if (this != std::addressof(other))
        {
            assign(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
        }
        return *this;
    }

    template <class InputIterator>
    constexpr void assign(InputIterator first, InputIterator last)
    {
        clear();
        std::copy(first, last, std::back_inserter(m_vec));
    }

    constexpr void assign(size_type n, const value_type& u)
    {
        clear();
        std::fill_n(std::back_inserter(m_vec), n, u);
    }

    constexpr void assign(std::initializer_list<value_type> il)
    {
        return assign(il.begin(), il.end()); 
    }

    // 5.4, destruction
    // We may not satisfy the remark that
    // the destructor shall be trivial if is_trivially_copyable_v<T> && is_default_constructible_v<T> is true.
    constexpr ~static_vector() = default;

    // iterators
    constexpr iterator begin() { return m_vec.begin(); }
    constexpr const_iterator begin() const { return m_vec.begin(); }
    constexpr iterator end() { return m_vec.end(); }
    constexpr const_iterator end() const { return m_vec.end(); }
    constexpr reverse_iterator rbegin() { return end(); }
    constexpr const_reverse_iterator rbegin() const { return end(); }
    constexpr reverse_iterator rend() { return begin(); }
    constexpr const_reverse_iterator rend() const { return begin(); }
    constexpr const_iterator cbegin() { return begin(); }
    constexpr const_iterator cend() { return end(); }
    constexpr const_reverse_iterator crbegin() { return rbegin(); }
    constexpr const_reverse_iterator crend() const { return rend(); }

    // 5.5, size/capacity:
    constexpr bool empty() const { return m_vec.empty(); }
    constexpr size_type size() const { return m_vec.size(); }
    static consteval size_type max_size() { return N; }
    static consteval size_type capacity() { return N; }

    constexpr void resize(size_type sz) 
    { 
        // Vector capacity is never reduced when resizing to smaller size 
        // because that would invalidate all iterators, rather than only the ones 
        // that would be invalidated by the equivalent sequence of pop_back() calls.
        check_overflow(sz);
        m_vec.resize(sz); 
    }

    constexpr void resize(size_type sz, const value_type& c) 
    { 
        check_overflow(sz);          
        m_vec.resize(sz, c); 
    }

    // 5.6, element and data access:
    constexpr reference operator[](size_type n) { return m_vec[n]; }
    constexpr const_reference operator[](size_type n) const { return m_vec[0]; }
    constexpr reference front() { return m_vec.front(); }
    constexpr const_reference front() const { return m_vec.front(); }
    constexpr reference back() { return m_vec.back(); }
    constexpr const_reference back() const { return m_vec.back(); }
    constexpr T* data() { return m_vec.data(); }
    constexpr const T* data() const { return m_vec.data(); }
    constexpr reference at(size_type n) { return m_vec.at(n); }
    constexpr const_reference at(size_type n) const { return m_vec.at(n); }

    // 5.7, modifiers:
    constexpr iterator insert(const_iterator position, const value_type& x)
    { return m_vec.insert(position, x); }

    constexpr iterator insert(const_iterator position, value_type&& x)
    { return m_vec.insert(position, std::move(x)); }

    constexpr iterator insert(const_iterator position, size_type n, const value_type& x)
    { return m_vec.insert(position, n, x); }

    template <class InputIterator>
    constexpr iterator insert(const_iterator position, InputIterator first, InputIterator last)
    { return m_vec.insert(position, first, last); }
    
    constexpr iterator insert(const_iterator position, std::initializer_list<value_type> il)
    { return m_vec.insert(position, il); }

    template <class... Args>
    constexpr iterator emplace(const_iterator position, Args&&... args)
    { return m_vec.emplace(position, (Args&&) args...); }

    template <class... Args>
    constexpr reference emplace_back(Args&&... args)
    { return m_vec.emplace_back((Args&&) args...); }

    constexpr void push_back(const value_type& x)
    { m_vec.push_back(x); }

    constexpr void push_back(value_type&& x)
    { m_vec.push_back(std::move(x)); }

    constexpr void pop_back()
    { m_vec.pop_back(); }

    constexpr iterator erase(const_iterator position)
    { return m_vec.erase(position); }

    constexpr iterator erase(const_iterator first, const_iterator last)
    { return m_vec.erase(first, last); }

    constexpr void clear() noexcept
    {
        // Leaves the capacity() of the vector unchanged.
        // (note: the standard's restriction on the changes to capacity is in the specification of vector::reserve)
        m_vec.clear();
    }

    constexpr void swap(static_vector& x) noexcept(std::is_nothrow_swappable_v<value_type>)
    { 
        auto small = this, large = &x;
        if (small->size() > large->size())
        {
            std::swap(small, large);
        }    

        auto common = small->size();

        // Swap common
        std::swap_ranges(small->begin(), small->end(), large->begin());

        // Move rest
        std::move(large->begin() + common, large->end(), std::back_inserter(*small));
        large->erase(large->begin() + common, large->end());
    }

    constexpr friend bool operator==(const static_vector& a, const static_vector& b)
    { return a.m_vec == b.m_vec; }

    // constexpr friend bool operator!=(const static_vector& a, const static_vector& b)

    constexpr friend bool operator<(const static_vector& a, const static_vector& b)
    { return a.m_vec < b.m_vec; }

    constexpr friend bool operator<=(const static_vector& a, const static_vector& b)
    { return a.m_vec <= b.m_vec; }

    constexpr friend bool operator>(const static_vector& a, const static_vector& b)
    { return a.m_vec > b.m_vec; }

    constexpr friend bool operator>=(const static_vector& a, const static_vector& b)
    { return a.m_vec >= b.m_vec; }

    constexpr friend auto operator<=>(const static_vector& a, const static_vector& b)
    { return a.m_vec <=> b.m_vec; };

    // Use std::ranges::swap, and ADL will help us find this function
    friend void swap(static_vector& lhs, static_vector& rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }

};

} // namespace cpp::collections


