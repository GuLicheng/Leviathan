/**
 * We implement a hashtable align with Python. There are some differences between Python.
 * 1. This hashtable is C++ style and the key and value will never split.
 * 2. Python dict use two array to save elements. One for indices(arr1) and another for elements(arr2). 
 * When removing an element, set value of arr1 to -2 and arr2 to nullptr(each element in Python is a PyObject*) is OK. 
 * But the element type in C++ does not require for pointer type. In such way, the iterator is not as efficient as Python.
*/
#pragma once

#include "hash_slot.hpp"
#include "../common.hpp"
#include "../associative_container_interface.hpp"

namespace leviathan::collections
{
    
template <typename KeyValue, 
    typename Hasher, 
    typename KeyEqual,
    typename Allocator,
    typename HashGenerator,
    bool Unique>
class py_hashtable : public reversible_container_interface,
                     public unordered_associative_container_insertion_interface
{
    static_assert(Unique, "Only support unique-key now.");

public:

    using hasher = HashFunction;
    using key_equal = KeyEqual;
    using allocator_type = Allocator;
    using key_type = typename KeyValue::key_type;
    using value_type = typename KeyValue::value_type;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

protected:

    static constexpr bool IsTransparent = detail::transparent<hasher, key_equal>;
    
    template <typename U>
    using key_arg_t = detail::key_arg<IsTransparent, U, key_type>;

    static constexpr bool CacheHashCode = detail::cache_hash_code<value_type>::value;

    using index_type = std::size_t;
    using slot_type = hash_cell<value_type, CacheHashCode>;
    using hash_generator_type = HashGenerator;

    using slot_allocator = typename std::allocator_traits<allocator_type>::template rebind_alloc<slot_type>;
    using slot_alloc_traits = std::allocator_traits<slot_allocator>;
    using indices_allocator = typename std::allocator_traits<allocator_type>::template rebind_alloc<index_type>;
    using indices_alloc_traits = std::allocator_traits<indices_allocator>;
    using alloc_traits = std::allocator_traits<allocator_type>;

    static constexpr index_type SlotUnused = static_cast<index_type>(-1);
    static constexpr index_type SlotDeleted = static_cast<index_type>(-2);
    static constexpr std::size_t default_hash_size = 8;

    static constexpr bool IsNothrowMoveConstruct = 
                std::is_nothrow_move_constructible_v<hasher> 
                && std::is_nothrow_move_constructible_v<key_equal> 
                && typename slot_alloc_traits::is_always_equal()
                && typename indices_alloc_traits::is_always_equal();

    static constexpr bool IsNothrowMoveAssign = 
                std::is_nothrow_move_assignable_v<hasher> 
                && std::is_nothrow_move_assignable_v<key_equal> 
                && typename slot_alloc_traits::is_always_equal()
                && typename indices_alloc_traits::is_always_equal();

    static constexpr bool IsNothrowSwap = 
                std::is_nothrow_swappable_v<hasher> 
                && std::is_nothrow_swappable_v<key_equal> 
                && typename slot_alloc_traits::is_always_equal()
                && typename indices_alloc_traits::is_always_equal();

    struct hash_iterator : bidirectional_iterator_interface
    {
        using link_type = py_hashtable*;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = value_type;
        using reference = std::conditional_t<std::is_same_v<key_type, value_type>, const value_type&, value_type&>;
        using difference_type = std::ptrdiff_t;

        link_type m_link;
        size_type m_idx;
    
        constexpr hash_iterator() = default;

        constexpr hash_iterator(const hash_iterator&) = default;

        constexpr hash_iterator(link_type link, std::size_t idx)  
            : m_link(link), m_idx(idx) { }

        constexpr bool operator==(const hash_iterator&) const = default;        

        constexpr reference operator*() const
        {
            auto pos = m_link->m_indices[m_idx];
            return m_link->m_slots[pos].value();
        }

        constexpr hash_iterator& operator++()
        {
            m_idx++;
            for (; m_idx < m_c->m_capacity && m_c->m_indices[m_idx] >= SlotDeleted; m_idx++);
            return *this;
        }

        using bidirectional_iterator_interface::operator++;

        constexpr hash_iterator& operator--()
        {
            m_idx--;
            for (; m_idx != static_cast<std::size_t>(-1) && m_c->m_indices[m_idx] >= SlotDeleted; m_idx--);
            return *this;   
        }

        using bidirectional_iterator_interface::operator--;
    };

public:

    using iterator = hash_iterator;
    using const_iterator = std::const_iterator<iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

protected:

    template <typename K>
    bool check_equal(std::size_t hash_code, std::size_t pos, const K& x) const 
    {
        // for integer with hash(x) = x, compare hash_code is equivalent to x == value
        // for std::string or other string types, compare hash_code first may faster
        if constexpr (CacheHashCode)
        {
            return hash_code == m_slots[pos].m_hash_code 
                && m_hk(x, KeyValue()(m_slots[pos].value()));
        }
        else
        {
            return m_hk(x, KeyValue()(m_slots[pos].value()));
        }
    }

    /**
     * @brief: Try insert element and return the position of index that the element inserted.
     * @return: (index, exits)
    */
    template <typename U>
    std::pair<std::size_t, bool> insert_with_hash_code(U&& u, std::size_t hash_code)
    {
        HashGenerator g { hash_code, m_capacity };
        auto offset = *g;

        slot_allocator alloc { m_alloc };

        while (1)
        {
            auto& state = m_indices[offset];

            if (state == SlotUnused)
            {
                // Insert element here
                if constexpr (CacheHashCode)
                {
                    slot_alloc_traits::construct(alloc, m_slots + m_used, hash_code, (U&&) u); 
                }
                else
                {
                    slot_alloc_traits::construct(alloc, m_slots + m_used, (U&&) u); 
                }
                state = m_used;
                m_used++;
                m_size++;
                return { offset, false };
            }

            // If the slot is deleted, we just skip it. 
            // If the slot is active, we try to compare it with element

            if (state != SlotDeleted && check_equal(hash_code, m_indices[offset], KeyValue()(u)))
            {
                // The element is exist.
                return { offset, true };
            }

            offset = g();

        }
        std::unreachable();
    }

    template <typename U>
    void rehash_insert_with_hash_code(U&& u, std::size_t hash_code)
    {
        HashGenerator g { hash_code, m_capacity };
        auto offset = *g;

        slot_allocator alloc { m_alloc };

        while (1)
        {
            auto& state = m_indices[offset];

            // We move the elements from old table to new table.
            // Each slot in new table only has two state: active or unused.
            if (state == SlotUnused)
            {
                if constexpr (CacheHashCode)
                {
                    slot_alloc_traits::construct(alloc, m_slots + m_used, hash_code, (U&&) u); 
                }
                else
                {
                    slot_alloc_traits::construct(alloc, m_slots + m_used, (U&&) u); 
                }
                state = m_used;
                m_used++;
                m_size++;
                return;
            }

            offset = g();

        }
        std::unreachable();
    } 

    template <typename U>
    std::pair<std::size_t, bool> insert_impl(U&& x)
    {
        rehash_and_growth_if_necessary();
        const auto hash = m_hk(KeyValue()(x));
        return insert_with_hash_code((U&&) x, hash);
    }

    void initialize(std::size_t new_capacity)
    {
        auto new_indices = detail::allocate<index_type>(m_alloc, new_capacity);
        auto new_slot = detail::allocate<slot_type>(m_alloc, new_capacity);
        std::uninitialized_fill_n(new_indices, new_capacity, SlotUnused);
        m_indices = new_indices;
        m_slots = new_slot;
        m_capacity = new_capacity;
    }

    void rehash_and_growth_if_necessary()
    {
        if (m_capacity == 0)
        {
            resize(default_hash_size);
            return;
        }

        constexpr double threshold = 2.0 / 3;

        const double factor = (double) m_used / m_capacity;
        if (factor > threshold)
        {
            const auto new_capacity = m_capacity << 1;
            resize(new_capacity);
        }
    }

    void resize(std::size_t new_capacity)
    {
        auto old_indices = m_indices;
        auto old_slots = m_slots;
        auto old_capacity = m_capacity;
        auto old_size = m_size;
        auto old_mused = m_used;

        initialize(new_capacity); 


        struct exception_helper
        {
            py_hashtable* t;

            index_type* indices;      
            slot_type* slots;         
            std::size_t size;         
            std::size_t capacity;     
            std::size_t used;         

            void stop()
            { t = nullptr; }

            ~exception_helper()
            {
                // Free memory and recover state
                if (t)
                {
                    t->clear();
                    t->m_indices = indices;
                    t->m_slots = slots;
                    t->m_size = size;
                    t->m_capacity = capacity;
                    t->m_used = used;
                }
            }
        };

        exception_helper helper { this, old_indices, old_slots, old_size, old_capacity, old_mused };

        // If std::is_nothrow_move_construable_v<T> is false, the `resize` will copy each 
        // element from old table to new table such as std::vector. To keep the state of 
        // container not be changed. We use exception_helper.
        m_size = 0;
        m_used = 0;
        slot_allocator alloc { m_alloc };
        for (std::size_t i = 0; i < old_capacity; ++i)
        {
            auto state = old_indices[i];
            if (state == SlotUnused)
            {
                continue;
            }
            else if (state == SlotDeleted)
            {
                slot_alloc_traits::destroy(alloc, old_slots + state);
            }
            else
            {
                std::size_t hash_code;
                auto pos = state;
                if constexpr (CacheHashCode)
                {
                    hash_code = old_slots[pos].m_hash_code;
                }
                else
                {
                    hash_code = m_hk(KeyValue()(old_slots[pos].value()));
                }

                rehash_insert_with_hash_code(std::move_if_noexcept(old_slots[pos].value()), hash_code);
                slot_alloc_traits::destroy(alloc, old_slots + pos);
            }
        }

        // If no exception is thrown, we stop helper and free the old memory.
        // Otherwise, helper's destructor will recover the state of container
        helper.stop();

        if (old_capacity)
        {
            assert(old_indices && old_slots && "these pointers should never be nullptr");
            detail::deallocate(m_alloc, old_indices, old_capacity);
            detail::deallocate(m_alloc, old_slots, old_capacity);
        }   
    }

    template <typename K>
    std::size_t find_slot_by_key(const K& x) const 
    {
        
        auto hash_code = m_hk(x);

        auto offset = find_slot_by_key_aux(x, hash_code);
        
        if (offset == m_capacity)
            return m_capacity;

        
        auto pos = m_indices[offset];
        if (pos == SlotUnused)
            return m_capacity;

        return offset;
    }

    template <typename K>
    size_type remove_by_key(const K& x)
    {
        auto idx = find_slot_by_key(x);
        if (idx == m_capacity)
            return 0;
        m_indices[idx] = SlotDeleted;
        m_size--;
        return 1;
    }

    template <typename I>
    iterator remove_by_iterator(I iter)
    {
        auto idx = iter.m_idx;
        m_indices[idx] = SlotDeleted;
        m_size--;
        return std::next(iterator(this, idx));
    }

    /**
     * @brief: Find the location of x.
     * @return: The offset of x in indices.
    */
    template <typename K>
    std::size_t find_slot_by_key_aux(const K& x, std::size_t hash_code) const 
    {
        if (m_capacity == 0)
        {
            return m_capacity;
        }
        
        HashGenerator g { hash_code, m_capacity };

        auto offset = *g;

        while (1)
        {
            auto pos = m_indices[offset];

            if (pos != SlotDeleted)
            {
                if (pos == SlotUnused || check_equal(hash_code, pos, x))
                    return offset;
            }

            offset = g();
        }
        std::unreachable();
    }


public:

    py_hashtable() 
        : py_hashtable(Hasher(), KeyEqual(), Allocator()) { }

    py_hashtable(const hasher& hash, const key_equal& ke, const allocator_type& alloc)
        : m_hk(hash, ke), m_alloc(alloc) { }

    py_hashtable(const allocator_type& alloc) : py_hashtable(Hasher(), KeyEqual(), alloc) { }

    ~py_hashtable()
    {
        clear();
    }

    void swap(py_hashtable& rhs) noexcept(IsNothrowSwap)
    {
        using std::swap;
        swap(m_capacity, rhs.m_capacity);
        swap(m_hk, rhs.m_hk);
        swap(m_indices, rhs.m_indices);
        swap(m_size, rhs.m_size);
        swap(m_slots, rhs.m_slots);
        swap(m_used, rhs.m_used);
        if constexpr (typename alloc_traits::propagate_on_container_swap())
        {
            swap(m_alloc, rhs.m_alloc);
        }
    }

    friend void swap(py_hashtable& lhs, py_hashtable& rhs)
        noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }

    std::size_t size() const
    {
        return m_size;
    }

    bool empty() const
    {
        return size() == 0;
    }

    std::size_t capacity() const
    {
        return m_capacity;
    }

    hasher hash_function() const
    {
        return static_cast<hasher>(m_hk);
    }

    key_equal key_eq() const
    {
        return static_cast<key_equal>(m_hk);
    }

    allocator_type get_allocator() const
    {
        return m_alloc;
    }

    const index_type *indices() const
    {
        return m_indices;
    }

    const slot_type *slots() const
    {
        return m_slots;
    }

    size_t max_size() const
    {
        return static_cast<std::size_t>(-1) - 2;
    }

    iterator begin()
    {
        std::size_t idx = 0;
        for (; idx < m_capacity && m_indices[idx] >= SlotDeleted; idx++);
        return { this, idx };
    }

    iterator end()
    {
        return {this, m_capacity};
    }

    const_iterator begin() const
    {
        return const_cast<py_hashtable &>(*this).begin();
    }

    const_iterator end() const
    {
        return const_cast<py_hashtable &>(*this).end();
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        if constexpr (detail::emplace_helper<value_type, Args...>::value)
        {
            // If there is only one arg and remove_cvref_t<decltype(arg)> is value_type
            // we just call insert_impl.
            // Since each element in our hashtable must be duplicated, so
            // if args... is from hashtable, it will not cause rehash
            // and we are not necessary to use value_handle.
            auto [idx, exist] = insert_impl((Args&&) args...);
            return { iterator(this, idx), !exist };
        }
        else
        {
            // // We construct value on stack and try to move it.
            // alignas(value_type) unsigned char raw[sizeof(value_type)];
            // value_type* slot = reinterpret_cast<value_type*>(&raw);
            // slot_allocator alloc { m_alloc };

            // // Whether the value is move successfully, the destructor should be invoked. 
            // // ~unique_ptr will destroy the object constructed.
            // auto deleter = [&](value_type* p) {
            //     slot_alloc_traits::destroy(alloc, p);
            // };
            // std::unique_ptr<value_type, decltype(deleter)> _ { slot, deleter };
            // slot_alloc_traits::construct(alloc, slot, (Args&&) args...);
            // auto [idx, exist] = insert_impl(std::move(*slot));
            value_handle<value_type, allocator_type> handle(m_alloc, (Args&&) args...);
            auto [idx, exist] = insert_impl(*handle);
            return { iterator(this, idx), !exist };
        }
    }

    template <typename... Args>
    iterator emplace_hint(const_iterator, Args &&...args)
    {
        return emplace((Args &&)args...).first;
    }

    template <typename K = key_type>
    bool contains(const key_arg_t<K> &x) const
    {
        return find(x) != end();
    }

    template <typename K = key_type>
    iterator find(const key_arg_t<K> &x)
    {
        return {this, find_slot_by_key(x)};
    }

    template <typename K = key_type>
    const_iterator find(const key_arg_t<K> &x) const
    {
        return const_cast<py_hashtable &>(*this).find(x);
    }

    iterator erase(iterator pos)
    {
        return remove_by_iterator(pos);
    }

    iterator erase(const_iterator pos)
    {
        return remove_by_iterator(pos);
    }

    // iterator erase( const_iterator first, const_iterator last );

    size_type erase(const key_type &x)
    {
        return remove_by_key(x);
    }

    template <typename K>
        requires (IsTransparent && 
            !std::is_convertible_v<iterator, K> && 
            !std::is_convertible_v<const_iterator, K>)
    size_type erase(K&& x)
    { return remove_by_key(x); }

    void clear()
    {
        if (m_capacity == 0)
        {
            assert(!m_size && !m_indices && !m_slots && !m_used);
            return;
        }

        // Destroy elements
        slot_allocator alloc { m_alloc };
        for (std::size_t i = 0; i < m_used; ++i)
        {
            slot_alloc_traits::destroy(alloc, m_slots + i);
        }

        // Free memory
        detail::deallocate(m_alloc, m_indices, m_capacity);
        detail::deallocate(m_alloc, m_slots, m_capacity);

        // Reset other member
        m_indices = nullptr;
        m_slots = nullptr;
        m_capacity = 0;
        m_size = 0;
        m_used = 0;
    }

private:

    [[no_unique_address]] allocator_type m_alloc;
    [[no_unique_address]] hash_key_equal<Hasher, KeyEqual> m_hk;

    index_type* m_indices = nullptr;      // store indices or state
    slot_type* m_slots = nullptr;         // store entries
    std::size_t m_size = 0;               // number of elements
    std::size_t m_capacity = 0;           // table capacity
    std::size_t m_used = 0;               // used slots, always point the end of m_slots

};


} // namespace leviathan::collections::hashtable

