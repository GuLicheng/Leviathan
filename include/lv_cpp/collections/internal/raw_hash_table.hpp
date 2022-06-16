/*
    Entries:
    
        +------------------------+
        |    |    |    |    |    |
        +------------------------+

    Allocates n * sizeof(T) bytes of uninitialized storage by calling 
    ::operator new(std::size_t) or ::operator new(std::size_t, std::align_val_t) (since C++17), 
    but it is unspecified when and how this function is called. The pointer hint may be used to 
    provide locality of reference: the allocator, if supported by the implementation, will attempt 
    to allocate the new memory block as close as possible to hint.

*/

#pragma once

// #include "config.hpp"
#include "hash_policy.hpp"
#include <lv_cpp/meta/template_info.hpp>
#include <memory>
#include <type_traits>
#include <limits>

namespace leviathan::collections
{

    namespace detail
    {
        // C++17 version
        // template <typename T, typename = void>
        // struct is_transparent : std::false_type { };
    
        // template <typename T>
        // struct is_transparent<T, std::void_t<typename T::is_transparent>> : std::true_type { };

        // C++20 simply use concept and require statement
        template <typename T> concept is_transparent = requires 
        { typename T::is_transparent; };

        // if IsTransparent is true, return K1, ohther 
        template <bool IsTransparent>
        struct key_arg_helper 
        {
            template <typename K1, typename K2>
            using type = K1;
        };

        template <>
        struct key_arg_helper<false> 
        {
            template <typename K1, typename K2>
            using type = K2;
        };

        template <bool IsTransparent, class K1, class K2>
        using key_arg = typename key_arg_helper<IsTransparent>::template type<K1, K2>;
    
        template <typename T, typename... Args>
        struct emplace_helper
        {
        private:
            constexpr static auto is_same_as_key = [](){
                if constexpr (sizeof...(Args) != 1)
                    return false;
                else
                    return std::conjunction_v<std::is_same<T, std::remove_cvref_t<Args>>...>;
            }();
        public:
            constexpr static bool value = is_same_as_key;
        };
    }

    // since the last 7 bits will extracted in H2, so remove last 7 bits here
    constexpr auto H1(int hash) noexcept
    {
        return hash >> 7;
    }

    // H2 capture last 7 bits
    constexpr std::uint8_t H2(int hash) noexcept
    {
        return hash & 0x7F;
    }

    enum class ctrl : int8_t {
        empty = -128,   // 0b10000000
        deleted = -2,   // 0b11111110
        sentinel = -1,  // 0b11111111
    };

    constexpr bool check_ctrl_is_empty(ctrl c) noexcept
    {
        return c == ctrl::empty;
    }

    constexpr bool check_ctrl_is_active(ctrl c) noexcept
    {
        return c >= static_cast<ctrl>(0);
    } 

    constexpr bool check_ctrl_is_deleted(ctrl c) noexcept
    { 
        return c == ctrl::deleted; 
    }
       
    constexpr bool check_ctrl_is_empty_or_deleted(ctrl c) noexcept
    { 
        return c < ctrl::sentinel; 
    }

    template <typename T, 
        typename HashFunction, 
        typename KeyEqual, 
        typename Allocator, 
        typename HashPolicy,
        bool Duplicate = true>
    class raw_hash_table 
    {

        static_assert(Duplicate, "Only support unique-key now.");
    
    public:

        using hasher = HashFunction;
        using key_equal = KeyEqual;
        using allocator_type = Allocator;
        using value_type = T;
        using key_type = typename HashPolicy::key_type;
        using policy_type = HashPolicy;

    private:

        using slot_type = typename HashPolicy::slot_type;
        using rebind_alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<slot_type>;
        using alloc_traits = std::allocator_traits<rebind_alloc>;
        inline constexpr static bool CacheHashCode = true;
        
        template <typename U>
        using key_arg_t = detail::key_arg<detail::is_transparent<hasher> && detail::is_transparent<key_equal>, U, key_type>;

        template <bool Const>
        struct hash_iterator
        {
            using link_type = std::conditional_t<Const, const raw_hash_table*, raw_hash_table*>;

            using iterator_category = std::forward_iterator_tag;
            using value_type = std::conditional_t<Const, const T, T>;
            using reference = value_type&;
            using difference_type = std::ptrdiff_t;

            link_type m_c;
            std::size_t m_idx;

            constexpr hash_iterator() noexcept = default;
            constexpr hash_iterator(const hash_iterator&) noexcept = default;
            constexpr hash_iterator(link_type c, std::size_t idx) noexcept 
                : m_c{ c }, m_idx{ idx } { }

            constexpr hash_iterator(const hash_iterator<!Const>& rhs) noexcept requires (Const)
                : m_c{ rhs.m_c }, m_idx{ rhs.m_idx } { }

            constexpr bool operator==(const hash_iterator&) const noexcept = default;

            constexpr reference operator*() const noexcept
            { return m_c->m_slots[m_idx].value(); }

            constexpr auto operator->() const noexcept
            { return std::addressof(**this); }

            constexpr hash_iterator& operator++() noexcept
            {
                m_idx++;
                for (; m_idx < m_c->m_capacity && check_ctrl_is_empty_or_deleted(m_c->m_ctrl[m_idx]); m_idx++);
                return *this;
            }

            constexpr hash_iterator operator++(int) noexcept
            {
                auto old = *this;
                ++*this;
                return old;
            }

        };


    public:

        using iterator = hash_iterator<false>;
        using const_iterator = hash_iterator<true>;
        using size_type = std::size_t;

        raw_hash_table() noexcept
            : m_hash{ }, m_ke{ }, m_alloc{ }, m_size{ }, m_capacity{ }, m_slots{ }, m_ctrl{ }
        {
        }

        std::size_t size() const noexcept
        { return m_size; }

        bool empty() const noexcept
        { return size() == 0; }

        std::size_t capacity() const noexcept
        { return m_capacity; }

        hasher hash_function() const noexcept(std::is_nothrow_copy_constructible_v<hasher>)
        { return m_hash; }

        key_equal key_eq() const noexcept(std::is_nothrow_copy_constructible_v<key_equal>)
        { return m_ke; }

        allocator_type get_allocator() const noexcept
        { return allocator_type(m_alloc); }

        float load_factor() const noexcept
        { return empty() ? 0.0 : (float)m_size / m_capacity; }

        float max_load_factor() const noexcept
        { return 1.0f; }

        void max_load_factor(float) 
        { /* Does nothing. */ }

        size_t max_size() const noexcept
        { return std::numeric_limits<size_t>::max(); }

        template <typename K = key_type>
        iterator find(const key_arg_t<K>& x)
        { return iterator(this, find_slot(x)); }

        template <typename K = key_type>
        const_iterator find(const key_arg_t<K>& x) const
        { return const_cast<raw_hash_table&>(*this).find(x); }

        std::pair<iterator, bool> insert(const key_type& x)
        { return emplace(x); }

        std::pair<iterator, bool> insert(key_type&& x)
        { return emplace(std::move(x)); }

        template <typename... Args>
        std::pair<iterator, bool> emplace(Args&&... args)
        {
            if constexpr (detail::emplace_helper<value_type, Args...>::value)
            {
                // If there is only one arg and remove_cvref_t<decltype(arg)> is value_type
                // we just call insert_impl
                auto [pos, exits] = insert_impl((Args&&) args...);
                return std::make_pair(iterator(this, pos), !exits);
            }
            else
            {
                // We first construct value on stack and try to move it.
                // Whether the value is move successfully, the destructor should be invoked. 
                alignas(slot_type) unsigned char raw[sizeof(slot_type)];
                slot_type* slot = reinterpret_cast<slot_type*>(&raw);
                alloc_traits::construct(m_alloc, slot, (Args&&) args...);
                auto [pos, exits] = insert_impl(std::move(*slot));
                alloc_traits::destroy(m_alloc, static_cast<slot_type*>(slot));
                return std::make_pair(iterator(this, pos), !exits);
            }
        }

        template <typename K = key_type>
        bool contains(const key_arg_t<K>& x) const
        { return find(x) != end(); }

        template <typename K = key_type>
        size_type erase(const key_arg_t<K>& x)
        { return remove_by_value(x); }

        iterator erase(iterator pos)
        { return remove_by_iter(pos); }

        iterator erase(const_iterator pos)
        { return remove_by_iter(pos); }

        iterator begin() noexcept 
        {
            std::size_t idx = 0;
            for (; idx < m_capacity && check_ctrl_is_empty_or_deleted(m_ctrl[idx]); ++idx);
            return iterator(this, idx);
        }

        iterator end() noexcept
        { return iterator(this, m_capacity); }

        const_iterator begin() const noexcept
        { return const_cast<raw_hash_table&>(*this).begin(); }

        const_iterator end() const noexcept
        { return const_cast<raw_hash_table&>(*this).end(); }

        const_iterator cbegin() const noexcept
        { return const_cast<raw_hash_table&>(*this).begin(); }

        const_iterator cend() const noexcept
        { return const_cast<raw_hash_table&>(*this).end(); }

        void show() const 
        {
            for (std::size_t i = 0; i < m_capacity; ++i)
            {
                if (check_ctrl_is_active(m_ctrl[i]))
                    std::cout << "i = " << i << " value = " << m_slots[i].value() << '\n';
            }
        }

    private:

        template <typename K>
        std::size_t find_slot(const K& x) const noexcept(std::is_nothrow_invocable_v<hasher, decltype(x)>) 
        {
            if (m_capacity == 0)
            {
                return m_capacity;
            }
            
            const auto hash = m_hash(x);
            typename HashPolicy::generator_type g{ H1(hash), m_capacity };

            auto pos = *g;

            while (1)
            {
                if (H2(hash) == (std::uint8_t)m_ctrl[pos] && m_ke(x, HashPolicy::get(m_slots[pos].value())))
                    return pos;
                if (m_ctrl[pos] == ctrl::empty)
                    return m_capacity;
                // pos = (pos + 1) & (m_capacity - 1);
                pos = g();
            }
            // std::unreachable()
        }

        // return:
        // (pos, exist)
        template <typename U>
        std::pair<std::size_t, bool> insert_impl(U&& x)
        {
            rehash_and_growth_if_necessary();
            const auto hash = m_hash(HashPolicy::get(x));
            return insert_with_hash((U&&) x, hash);
        }

        template <typename U>
        std::pair<std::size_t, bool> insert_with_hash(U&& x, std::size_t hash)
        {
            typename HashPolicy::generator_type g{ H1(hash), m_capacity };
            auto pos = *g;
            while (1)
            {
                if (H2(hash) == (std::uint8_t)m_ctrl[pos] && m_ke(x, HashPolicy::get(m_slots[pos].value())))
                    return std::make_pair(pos, true);
                
                if (check_ctrl_is_empty_or_deleted(m_ctrl[pos]))
                {
                    if (check_ctrl_is_deleted(m_ctrl[pos]))
                    {
                        alloc_traits::destroy(m_alloc, m_slots + pos);
                    }
                    if constexpr (CacheHashCode)
                    {
                        alloc_traits::construct(m_alloc, m_slots + pos, hash, (U&&) x); 
                    }
                    else
                    {
                        alloc_traits::construct(m_alloc, m_slots + pos, (U&&) x); 
                    }

                    m_ctrl[pos] = static_cast<ctrl>(H2(hash));
                    m_size++;
                    return std::make_pair(pos, false);
                }
                pos = g();
            }
            // std::unreachable()
        }


        template <typename K>
        auto remove_by_value(const K& x)
        {
            auto idx = find_slot(x);
            if (idx == m_capacity)
                return 0;
            m_ctrl[idx] = ctrl::deleted;
            m_size--;
            return 1;
        }

        template <typename I>
        iterator remove_by_iter(I iter)
        {
            auto pos = iter.m_idx;
            m_ctrl[pos] = ctrl::deleted;
            m_size--;
            return std::next(iterator(this, pos));
        }

        // use for resize and constructor
        void initialize(std::size_t new_capacity)
        {
            // in this routine, std::out_of_memory may thrown
            // we don't need to catch since each member is not changed before.
            auto new_ctrl = detail::allocate<ctrl>(m_alloc, new_capacity);
            auto new_slot = detail::allocate<slot_type>(m_alloc, new_capacity);
            std::uninitialized_fill_n(new_ctrl, new_capacity, ctrl::empty);
            m_ctrl = new_ctrl;
            m_slots = new_slot;
            m_capacity = new_capacity;
        }

        void rehash_and_growth_if_necessary()
        {
            if (m_capacity == 0)
            {
                resize(1);
                return;
            }

            const double factor = (double) m_size / m_capacity;
            if (HashPolicy::need_expand(factor))
            {
                const auto new_capacity = HashPolicy::next_capacity(m_capacity);
                resize(new_capacity);
            }
        }

        void resize(std::size_t new_capacity)
        {
            auto old_ctrl = m_ctrl;
            auto old_slot = m_slots;
            auto old_capacity = m_capacity;

            initialize(new_capacity); 

            // If we don't cache hash code, we should use m_hash to get hash code again for each active value.
            // If std::is_nothrow_move_construable_v<T>, the rehash should be noexcept and we can call std::move for
            // each element otherwise we must copy each element so that if an exception is thrown, the state of 
            // container is not changed. 
            m_size = 0;
            for (std::size_t i = 0; i < old_capacity; ++i)
            {
                const auto state = old_ctrl[i];
                if (check_ctrl_is_deleted(state))
                {
                    alloc_traits::destroy(m_alloc, old_slot + i);
                }
                else if (check_ctrl_is_active(state))
                {
                    // try insert
                    std::size_t hash_code;
                    if constexpr (CacheHashCode)
                    {
                        hash_code = old_slot[i].m_hash_code;
                    }
                    else
                    {
                        hash_code = m_hash(HashPolicy::get(old_slot[i].value()));
                    }
                    insert_with_hash(std::move_if_noexcept(old_slot[i].value()), hash_code);
                }
            }

            if (old_capacity)
            {
                assert(old_ctrl && "this pointer will never be nullptr");
                detail::deallocate(m_alloc, old_ctrl, old_capacity);
                detail::deallocate(m_alloc, old_slot, old_capacity);
            }   
        }


        [[no_unique_address]] hasher m_hash;
        [[no_unique_address]] key_equal m_ke;
        [[no_unique_address]] rebind_alloc m_alloc;
    
        ctrl* m_ctrl; 
        slot_type* m_slots; // store entries
        std::size_t m_size;  // number of elements
        std::size_t m_capacity; // table capacity

    };


    template <typename T, 
        typename HashFunction = std::hash<T>, 
        typename KeyEqual = std::equal_to<>, 
        typename Allocator = std::allocator<T>>
    class hash_set : public raw_hash_table<T, HashFunction, KeyEqual, Allocator, default_hash_set_policy<T>> { };




}











