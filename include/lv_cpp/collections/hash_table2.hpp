/* 
    https://github.com/python/cpython/blob/main/Objects/dictobject.c
    We simply implement by template
*/

#pragma once

/*
Something defined in dictobject.c

+---------------------+
| dk_refcnt           |
| dk_log2_size        |
| dk_log2_index_bytes |
| dk_kind             |
| dk_usable           |
| dk_nentries         |
+---------------------+
| dk_indices[]        |  
|                     |
+---------------------+
| dk_entries[]        |
|                     |
+---------------------+
*/

#include "config.hpp"

// #include <lv_cpp/collections/config.hpp>
#include "hash_table.hpp"

#include <type_traits>
#include <functional> // std::hash
#include <bit>
#include <assert.h>



namespace leviathan::collections
{

    namespace detail
    {

        enum class state : unsigned char
        {
            empty, active, deleted
        };

    } // namespace detail


    template <typename T, 
        typename HashFunction, 
        typename KeyEqual, 
        typename Allocator, 
        typename Config, 
        typename HashPolicy = detail::python_dict<>,
        bool StoreHashCode = detail::cache_hash_code<T>::value,
        bool Duplicate = true,
        std::size_t MinSize = 8>
    class hash_table_impl2 : public Config
    {

        static_assert(Duplicate, "Don't support multi-key");

        using storage_type = detail::storage_impl<T, StoreHashCode>;

        // simply use std::default_sentinel_t as our sentinel type
        // but define another sentinel and monostate may be better
        using entry_type = storage_type;
        using vector_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<entry_type>;
        using self_type = hash_table_impl2;

        using allocator_traits = std::allocator_traits<vector_allocator_type>;

        template <bool Const>
        struct hash_iterator
        {

            using link_type = std::conditional_t<Const, const hash_table_impl2*, hash_table_impl2*>;

            using value_type = std::conditional_t<Const, const T, T>;
            using reference = value_type&;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            link_type m_c;
            std::size_t m_idx;

            constexpr hash_iterator() noexcept = default;
            constexpr hash_iterator(const hash_iterator&) noexcept = default;
            constexpr hash_iterator(const hash_iterator<!Const>& rhs) noexcept requires (Const)
                : m_c{ rhs.m_c }, m_idx{ rhs.m_idx } { }

            constexpr hash_iterator(link_type c, std::size_t idx) noexcept
                : m_c{ c }, m_idx{ idx } { }

            constexpr hash_iterator& operator++() noexcept
            {
                do ++m_idx; while (!m_c->check_item_is_active(m_idx));  // skip empty and stop in active or sentinel
                return *this;
            } 

            constexpr hash_iterator operator++(int) noexcept
            {
                auto old = *this;
                ++ *this;
                return old;
            }

            constexpr bool operator==(const hash_iterator& rhs) const noexcept = default;

            constexpr reference operator*() const noexcept 
            { return m_c->m_entries[m_idx].value(); }
            
            constexpr auto operator->() const noexcept 
            { return &(this->operator*()); }

        };


    public:

        using iterator = hash_iterator<false>;
        using const_iterator = hash_iterator<true>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = reverse_iterator;

		using typename Config::allocator_type;
		using typename Config::value_type;
		using typename Config::key_type;
		using typename Config::size_type;
		using typename Config::hasher;
		using typename Config::key_equal;

        using Config::config;

        hash_table_impl2() noexcept
            : m_size{ 0 }, m_hash{ }, m_ke{ }  
        {
            resize_and_make_sentinel(MinSize + 1);
        }

        ~hash_table_impl2() noexcept
        { reset(); }

        std::size_t size() const noexcept
        { return m_size; }

        bool empty() const noexcept
        { return size() == 0; }

        void clear() noexcept
        {
            reset();
            resize_and_make_sentinel(MinSize + 1);
        }

        std::pair<iterator, bool> insert(const value_type& x) 
        {
            auto [slot, exist] = insert_unique(x);
            return { iterator(this, slot), !exist };
        }

        std::pair<iterator, bool> insert(value_type&& x) 
        {
            auto [slot, exist] = insert_unique(std::move(x));
            return { iterator(this, slot), !exist };
        }

        template <typename... Args>
        std::pair<iterator, bool> emplace(Args&&... args) 
        {
            value_type x { args... };
            return insert(std::move(x));
        }

        iterator insert(const_iterator, const value_type& x) 
        { return insert(x).first; }

        iterator insert(const_iterator, value_type&& x) 
        { return insert(std::move(x)).first; }

        template <typename K> iterator find(const K& x) noexcept 
        {
            const auto hash = m_hash(x);
            auto [slot, exist] = find_entry(x, hash);
            return exist == detail::state::active ? iterator(this, slot) : end();
        }

        template <typename K> 
        auto& operator[](K&& k) requires (config == config_type::map)
        { return insert(std::make_pair((K&&) k, typename Config::mapped_type())).first->second; }	

        template <typename K> const_iterator find(const K& x) const noexcept 
        { return const_cast<self_type&>(*this).find(x); }

        template <typename K> bool contains(const K& x) const noexcept 
        { 
            const auto hash = m_hash(x);
            auto [slot, exist] = find_entry(x, hash);
            return exist == detail::state::active;
        } 

        iterator erase(const_iterator pos) 
        { return remove_item_by_iterator(pos); }

        iterator erase(iterator pos) 
        { return remove_item_by_iterator(pos); }

        template <typename K> size_type erase(const K& x) 
        { return remove_item(x); }

        iterator begin() noexcept 
        {
            std::size_t idx = 0;
            for (; idx < table_size() && !check_item_is_active(idx); idx++);
            return { this, idx };
        }
        
        iterator end() noexcept 
        { return { this, table_size() }; }

        const_iterator begin() const noexcept 
        { return const_cast<self_type&>(*this).begin(); }

        const_iterator end() const noexcept 
        { return const_cast<self_type&>(*this).end(); }

        const_iterator cbegin() const noexcept 
        { return begin(); }

        // auto global_begin() { return m_entries.begin(); }
        // auto global_end() { return m_entries.end(); }
        void show_state() const {
            for (int i = 0; i < m_states.size(); ++i)
                std::cout << "(" << i << " ," << (int)m_states[i] << ") "; 
            std::endl(std::cout);
        }

    private:
        std::vector<entry_type, vector_allocator_type> m_entries;
        std::vector<detail::state> m_states;
        std::size_t m_size; 
        [[no_unique_address]] hasher m_hash;
        [[no_unique_address]] key_equal m_ke;

        
        void resize_and_make_sentinel(std::size_t sz)
        {
            m_entries.reserve(sz);
            m_states.resize(sz);
        }

        void reset() noexcept
        {
            auto alloc = m_entries.get_allocator();
            for (std::size_t i = 0; i < m_states.size(); ++i)
            {
                if (!check_item_is_empty(i))
                {
                    allocator_traits::destroy(alloc, std::addressof(m_entries[i]));
                }
            }
            m_size = 0;
            m_states.clear();
            m_entries.clear();
        }

        template <typename U>
        std::pair<std::size_t, bool> insert_unique(U&& x)
        {

            rehash_and_grow_if_necessary();

            std::size_t hash = m_hash(Config::get_key(x));
            auto [slot, exits] = find_entry(Config::get_key(x), hash);

            if (exits == detail::state::active)
            {
                return { slot, false };
            }

            auto alloc = m_entries.get_allocator();

            if (exits == detail::state::deleted)
            {
                // try destroy old item and construct new item
                allocator_traits::destroy(alloc, std::addressof(m_entries[slot]));
            }

            // try construct new item
            if constexpr (StoreHashCode)
            {
                allocator_traits::construct(alloc, std::addressof(m_entries[slot]), hash, (U&&)x);
            }
            else
            {
                allocator_traits::construct(alloc, std::addressof(m_entries[slot]), x);
            }

            // update state and size
            m_states[slot] = detail::state::active;
            m_size++;
            
            return { slot, true };
        }

        bool check_item_is_empty(std::size_t idx) const noexcept
        { return m_states[idx] == detail::state::empty; }

        bool check_item_is_active(std::size_t idx) const noexcept
        { return m_states[idx] == detail::state::active; }

        bool check_item_is_deleted(std::size_t idx) const noexcept
        { return m_states[idx] == detail::state::deleted; }

        std::size_t table_size() const noexcept
        { return m_states.size() - 1; }

        /*
         * Return: (slot, state)
         *  slot: the index of item
         *  state:
         *      active: indicate the found item is exits
         *      deleted: indicate the found item is not exits
         *      empty: indicate the found item is not exits
         */
        template <typename K> 
        std::pair<std::size_t, detail::state> find_entry(const K& x, std::size_t hash) const noexcept // assume hasher is noexcept
        {

            detail::state exist;

            HashPolicy hash_generator { hash, table_size() }; // hash_init, mask

            // std::cout << m_states.size() << '\n';
            // this->show_state();

            auto slot = hash_generator.first();

            while (1)
            {
                // deleted item can be used for construct new item
                if (check_item_is_deleted(slot))
                {
                    exist = detail::state::deleted;
                    break;
                }

                // empty item can be used for construct new item directly
                if (check_item_is_empty(slot))
                {
                    exist = detail::state::empty;
                    break;
                }

                // active state and the key of item is equal to x
                if (m_ke(Config::get_key(m_entries[slot].value()), x))
                {
                    exist = detail::state::active;
                    break;
                }
                slot = hash_generator();
            }
            return { slot, exist };
        }

        // this method is used for rehash, the state cannot be deleted in this situation
        static std::size_t find_entry_only_with_hash_code(std::size_t hash, std::vector<detail::state>& new_state) noexcept // assume hasher is noexcept
        {
            std::cout << hash << '\n';
            HashPolicy hash_generator { hash, new_state.size() - 1 }; // hash_init, mask
            auto slot = hash_generator.first();
            for (; new_state[slot] != detail::state::empty; slot = hash_generator());
            return slot;
        }

        void rehash_and_grow_if_necessary()
        {
            // this method does not change the number of elements

            double load_factor = (double)m_size / table_size();

            if (!HashPolicy::need_expand(load_factor))
            {
                return; 
            }

            const auto next_capacity = growth_rate() + 1;
            
            std::vector<entry_type, vector_allocator_type> new_entries;
            std::vector<detail::state> new_states;

            new_entries.reserve(next_capacity);
            new_states.resize(next_capacity);

            for (std::size_t i{}; i < table_size(); ++i)
            {
                if (check_item_is_active(i))
                {
                    // gain hash_code
                    std::size_t hash_code;
                    if constexpr (StoreHashCode)
                    {
                        hash_code = m_entries[i].m_hash_code;
                    }
                    else
                    {
                        hash_code = m_hash(Config::get_key(m_entries[i].value()));
                    }

                    auto slot = find_entry_only_with_hash_code(hash_code, new_states);
                    // std::cout << "hash: " << hash_code << " slot = " << slot << "\n";
                    auto alloc = new_entries.get_allocator();

                    if (check_item_is_deleted(slot))
                    {
                        allocator_traits::destroy(alloc, std::addressof(m_entries[i]));
                    }

                    allocator_traits::construct(
                        alloc, 
                        std::addressof(new_entries[i]),
                        std::move_if_noexcept(m_entries[i]));
                    new_states[slot] = detail::state::active;

                }
                else if (check_item_is_deleted(i))
                {
                    // try destroy
                    auto alloc = m_entries.get_allocator();
                    allocator_traits::destroy(alloc, std::addressof(m_entries[i]));
                }
                else
                    ;  // empty and do nothing
                      
            }

            m_entries = std::move(new_entries);
            m_states = std::move(new_states);

            this->show_state();
        }

        std::size_t growth_rate() const noexcept
        { return HashPolicy::next_capacity(table_size()); }

        template <typename K>
        size_type remove_item(const K& x)
        {
            const auto hash = m_hash(x);
            auto [slot, exist] = find_entry(x, hash);
            if (exist == detail::state::active)
            {
                m_states[slot] = detail::state::deleted;
                m_size--;
                return 1;
            }
            return 0;
        }

        template <typename I>
        iterator remove_item_by_iterator(I iter)
        {
            auto slot = iter.m_idx;
            m_states[slot] = detail::state::deleted;
            m_size--;
            return ++iter;
        }

    };

    template <typename T, 
        typename HashFunction = std::hash<T>, 
        typename KeyEqual = std::equal_to<void>, 
        typename Allocator = std::allocator<T>>
    using hash_table2 = hash_table_impl2<
        T, 
        HashFunction, 
        KeyEqual, 
        Allocator, 
        hash_set_config<T, HashFunction, KeyEqual, Allocator>>;

    template <typename K, typename V, 
        typename HashFunction = std::hash<K>, 
        typename KeyEqual = std::equal_to<void>, 
        typename Allocator = std::allocator<std::pair<K, V>>>
    using hash_map2 = hash_table_impl2<
        std::pair<K, V>, 
        HashFunction, 
        KeyEqual, 
        Allocator, 
        hash_map_config<K, V, HashFunction, KeyEqual, Allocator>>;

}




