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

#include "hash_policy.hpp"
#include "meta_helper.hpp"
#include <lv_cpp/meta/template_info.hpp>
#include <memory>
#include <type_traits>
#include <limits>


namespace leviathan::collections
{

    struct auto_hash { explicit auto_hash() = default; };


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
        active = 0
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
        bool Duplicate = false>
    class raw_hash_table 
    {

        static_assert(!Duplicate, "Only support unique-key now.");
    
    public:

        using hasher = HashFunction;
        using key_equal = KeyEqual;
        using allocator_type = Allocator;
        using value_type = T;
        using key_type = typename HashPolicy::key_type;
        using policy_type = HashPolicy;
        using reference = value_type&;
        using const_reference = const value_type&;
        using difference_type = std::ptrdiff_t;

    private:

        using slot_type = typename HashPolicy::slot_type;
        using alloc_traits = std::allocator_traits<allocator_type>;

        using slot_alloc = typename std::allocator_traits<Allocator>::template rebind_alloc<slot_type>;
        using slot_alloc_traits = std::allocator_traits<slot_alloc>;

        constexpr static bool CacheHashCode = HashPolicy::cache_hash_code;

        constexpr static bool IsMap = HashPolicy::is_map;

        constexpr static bool IsNoexceptMoveConstruct = 
                std::is_nothrow_move_constructible_v<hasher>
             && std::is_nothrow_move_constructible_v<key_equal>
             && std::is_nothrow_move_constructible_v<value_type> 
             && std::is_nothrow_move_constructible_v<allocator_type>; 

        constexpr static bool IsNoexceptMoveAssignable = 
                IsNoexceptMoveConstruct
            && std::is_nothrow_move_assignable_v<hasher>
            && std::is_nothrow_move_assignable_v<key_equal>
            && std::is_nothrow_move_assignable_v<value_type>
            && std::is_nothrow_move_assignable_v<allocator_type>
            && []() {
                if constexpr (typename alloc_traits::propagate_on_container_move_assignment())
                    return true;
                else if constexpr (typename alloc_traits::is_always_equal())
                    return true;
                else
                    return false; // in this routine, assign_from may invoked 
            }();
     

        // we must evaluate IsTransparent first
        constexpr static bool IsTransparent = detail::is_transparent<hasher> && detail::is_transparent<key_equal>;
        template <typename U>
        using key_arg_t = detail::key_arg<IsTransparent, U, key_type>;

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
        using pointer = typename alloc_traits::pointer;
        using const_pointer = typename alloc_traits::const_pointer;


        raw_hash_table() 
            : m_hash{ }, m_ke{ }, m_alloc{ }, m_size{ }, m_capacity{ }, m_slots{ }, m_ctrl{ }
        {
        }

        raw_hash_table(hasher hash, key_equal ke, allocator_type alloc)
            : m_hash{ hash }, m_ke{ ke }, m_alloc{ alloc }, m_size{ }, m_capacity{ }, m_slots{ }, m_ctrl{ }
        {
        }

        raw_hash_table(const raw_hash_table& rhs) 
            : m_hash{ rhs.m_hash }, 
              m_ke{ rhs.m_ke }, 
              m_alloc{ alloc_traits::select_on_container_copy_construction(rhs.m_alloc) },
              m_size{ },
              m_capacity{ },
              m_slots{ },
              m_ctrl{ }
        {
            try
            {
                assign_from(rhs.begin(), rhs.end());
            }
            catch(...)
            {
                reset();
                throw;
            }
        }

		raw_hash_table(raw_hash_table&& rhs) 
		noexcept(IsNoexceptMoveConstruct)
			: m_hash{ std::move(rhs.m_hash) },
              m_ke{ std::move(rhs.m_ke) }, 
              m_alloc{ std::move(rhs.m_alloc) }, 
              m_size{ std::exchange(rhs.m_size, 0) }, 
              m_capacity{ std::exchange(rhs.m_capacity, 0) },
              m_slots{ std::exchange(rhs.m_slots, nullptr) },
              m_ctrl{ std::exchange(rhs.m_ctrl, nullptr) }
		{
        }

		void swap(raw_hash_table& rhs)
		{
			if (this != std::addressof(rhs))
			{
				if constexpr (typename alloc_traits::propagate_on_container_swap())
				{
					// std::swap(impl and alloc)
					swap_impl(rhs);
					std::swap(m_alloc, rhs.m_alloc);
				}
				else if (typename alloc_traits::is_always_equal()
					|| m_alloc == rhs.m_alloc)
				{
					swap_impl(rhs);
				}
				else
				{
					// Undefined Behaviour
					throw std::runtime_error("Undefined Behaviour");
				}
			}
		}

		raw_hash_table& operator=(const raw_hash_table& rhs)
		{
			if (std::addressof(rhs) != this)
			{
				// std::true_type
				if constexpr (typename std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment())
				{
					if (m_alloc != rhs.m_alloc)
					{
						// clear_and_deallocate_memory()
						reset();
					}
					m_alloc = rhs.m_alloc;
				}
				// assign_from(rhs.begin(), rhs.end());
				try
				{
					assign_from(rhs.cbegin(), rhs.cend());
				}
				catch (...)
				{
					reset();
					throw;
				}
			}
			return *this;
		}

		raw_hash_table& operator=(raw_hash_table&& rhs) 
		noexcept(IsNoexceptMoveAssignable)
		{
			if (this != std::addressof(rhs))
			{
				if constexpr (typename alloc_traits::propagate_on_container_move_assignment())
				{
					// clear_and_deallocate_memory
					// move alloc and impl
					reset();
					m_alloc = std::move(rhs.m_alloc);
					move_impl_and_reset_other(rhs);
				}
				else if (typename alloc_traits::is_always_equal() || m_alloc == rhs.m_alloc)
				{
					// clear_and_deallocate_memory()
					// impl = move(rhs.impl)
                    reset();
					move_impl_and_reset_other(rhs);
				}
				else
				{
					// assign(move_iter(rhs.begin()), move_iter(rhs.end()));
					assign_from(std::make_move_iterator(rhs.begin()), std::make_move_iterator(rhs.end()));
				}
			}
			return *this;
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
        { return (float)HashPolicy::factor; }

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

        template <typename K = key_type>
        bool contains(const key_arg_t<K>& x) const
        { return find(x) != end();  }

        std::pair<iterator, bool> insert(const key_type& x)
        { return emplace(x); }

        std::pair<iterator, bool> insert(key_type&& x)
        { return emplace(std::move(x)); }

        iterator insert(const_iterator, const value_type& x) 
        { return insert(x).first; }

        iterator insert(const_iterator, value_type&& x) 
        { return insert(std::move(x)).first; }

        template <class... Args>
        iterator emplace_hint(const_iterator, Args&&... args)
        { return emplace((Args&&) args...).first; }

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
                alignas(T) unsigned char raw[sizeof(T)];
                T* slot = reinterpret_cast<T*>(&raw);
                slot_alloc alloc { m_alloc };
                slot_alloc_traits::construct(alloc, slot, (Args&&) args...);

                // insert may invoke `rehash` and std::bad_alloc may be thrown
                // ~unique_ptr will destroy the object constructed above
                auto deleter = [&](T* p) {
                    slot_alloc_traits::destroy(alloc, p);
                };
                std::unique_ptr<T, decltype(deleter)> _ { slot, deleter };
                auto [pos, exits] = insert_impl(std::move(*slot));
                return std::make_pair(iterator(this, pos), !exits);
            }
        }

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

        void clear()
        { reset(); }

        ~raw_hash_table() 
        { reset(); }

    private:

        void reset()
        {

            if (!m_capacity)
            {
                assert(!m_ctrl && !m_slots && "ctrl and slots must be nullptr when hash table is empty.");
                return;
            }

            for (std::size_t i = 0; i < m_capacity; ++i)
            {
                if (!check_ctrl_is_empty(m_ctrl[i]))
                {
                    alloc_traits::destroy(m_alloc, m_slots + i);
                }
            }
            assert(m_ctrl && m_slots);
            detail::deallocate(m_alloc, m_ctrl, m_capacity);
            detail::deallocate(m_alloc, m_slots, m_capacity);
            m_capacity = 0;
            m_size = 0;
            m_slots = nullptr;
            m_ctrl = nullptr;
        }

        void swap_impl(raw_hash_table& rhs)
        {
            std::swap(m_ctrl, rhs.m_ctrl);
            std::swap(m_slots, rhs.m_slots);
            std::swap(m_size, rhs.m_size);
            std::swap(m_capacity, rhs.m_capacity);
            std::swap(m_hash, rhs.m_hash);
            std::swap(m_ke, rhs.m_ke);
        }

        template <typename I, typename S>
        void assign_from(I first, S last)
        {
            for (auto iter = first; iter != last; ++iter)
                insert(*iter);
        }

        void move_impl_and_reset_other(raw_hash_table& rhs)
        {
            m_hash = std::move(rhs.m_hash);
            m_ke = std::move(rhs.m_ke);
            m_ctrl = std::exchange(rhs.m_ctrl, nullptr);
            m_slots = std::exchange(rhs.m_slots, nullptr);
            m_size = std::exchange(rhs.m_size, 0);
            m_capacity = std::exchange(rhs.m_capacity, 0);
        }

        template <typename K>
        bool check_equal(std::size_t hash, std::size_t pos, const K& x) const noexcept
        {
            // for integer with hash(x) = x, compare hash_code is equivalent to x == value
            // for std::string or other string type, compare hash_code first may faster
            if constexpr (CacheHashCode)
            {
                return H2(hash) == (std::uint8_t)m_ctrl[pos]
                    && hash == m_slots[pos].m_hash_code 
                    && m_ke(x, HashPolicy::get(m_slots[pos].value()));
            }
            else
            {
                return H2(hash) == (std::uint8_t)m_ctrl[pos] 
                    && m_ke(x, HashPolicy::get(m_slots[pos].value()));
            }
        }

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
                // if (H2(hash) == (std::uint8_t)m_ctrl[pos] && m_ke(x, HashPolicy::get(m_slots[pos].value())))
                if (check_equal(hash, pos, x))
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

            slot_alloc alloc { m_alloc };

            while (1)
            {
                // if (H2(hash) == (std::uint8_t)m_ctrl[pos] && m_ke(x, HashPolicy::get(m_slots[pos].value())))
                if (check_equal(hash, pos, HashPolicy::get(x)))
                    return std::make_pair(pos, true);
                
                if (check_ctrl_is_empty_or_deleted(m_ctrl[pos]))
                {
                    if (check_ctrl_is_deleted(m_ctrl[pos]))
                    {
                        slot_alloc_traits::destroy(alloc, m_slots + pos);
                    }
                    if constexpr (CacheHashCode)
                    {
                        slot_alloc_traits::construct(alloc, m_slots + pos, hash, (U&&) x); 
                    }
                    else
                    {
                        slot_alloc_traits::construct(alloc, m_slots + pos, (U&&) x); 
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
            // in this routine, std::bad_alloc may thrown
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
                resize(8);
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
            slot_alloc alloc { m_alloc };
            for (std::size_t i = 0; i < old_capacity; ++i)
            {
                const auto state = old_ctrl[i];
                if (check_ctrl_is_deleted(state))
                {
                    slot_alloc_traits::destroy(alloc, old_slot + i); // destroy deleted element
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
                    slot_alloc_traits::destroy(alloc, old_slot + i); // element after moved also should be destroyed 
                }
            }

            if (old_capacity)
            {
                assert(old_ctrl && old_slot && "these pointers should never be nullptr");
                detail::deallocate(m_alloc, old_ctrl, old_capacity);
                detail::deallocate(m_alloc, old_slot, old_capacity);
            }   
        }


        [[no_unique_address]] hasher m_hash;
        [[no_unique_address]] key_equal m_ke;
        [[no_unique_address]] allocator_type m_alloc;
    
        ctrl* m_ctrl; 
        slot_type* m_slots; // store entries
        std::size_t m_size;  // number of elements
        std::size_t m_capacity; // table capacity

    };



    template <typename T, 
        typename HashFunction = std::hash<auto_hash>, 
        typename KeyEqual = std::equal_to<>, 
        typename Allocator = std::allocator<T>>
    using hash_set = raw_hash_table<T, HashFunction, KeyEqual, Allocator, default_hash_set_policy<T>>;


    template <typename K, typename V, 
        typename HashFunction = std::hash<auto_hash>, 
        typename KeyEqual = std::equal_to<>, 
        typename Allocator = std::allocator<std::pair<const K, V>>>
    class hash_map : public raw_hash_table<std::pair<const K, V>, HashFunction, KeyEqual, Allocator, default_hash_map_policy<const K, V>>
    {
    public:
        using mapped_type = V;
        using key_type = K;

        V& operator[](const K& x)
        { return this->emplace(x, V()).first->second; }

        V& operator[](K&& x)
        { return this->emplace(std::move(x), V()).first->second; }

    };

}



namespace std
{
    template <>
    struct hash<::leviathan::collections::auto_hash>
    {
        using is_transparent = void;

        template <typename T>
        constexpr auto operator()(const T& x) const noexcept(std::is_nothrow_invocable_v<std::hash<T>, const T&>)
        { return std::hash<T>()(x); }
    };

    template <typename T, 
        typename HashFunction, 
        typename KeyEqual, 
        typename Allocator, 
        typename HashPolicy,
        bool Duplicate>
    void swap(
        ::leviathan::collections::raw_hash_table<T, HashFunction, KeyEqual, Allocator, HashPolicy, Duplicate>& lhs,
        ::leviathan::collections::raw_hash_table<T, HashFunction, KeyEqual, Allocator, HashPolicy, Duplicate>& rhs)
    {
        lhs.swap(rhs);
    }

    template <typename K, typename V, 
        typename HashFunction, 
        typename KeyEqual, 
        typename Allocator>
    void swap(
        ::leviathan::collections::hash_map<K, V, HashFunction, KeyEqual, Allocator>& lhs,
        ::leviathan::collections::hash_map<K, V, HashFunction, KeyEqual, Allocator>& rhs)
    {
        lhs.swap(rhs);
    }


}




