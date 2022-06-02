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

#include <functional> // std::hash
#include <variant>

namespace leviathan::collections
{

    namespace detail
    {
        // j = ((5*j) + 1) mod 2**i 
        constexpr std::size_t next_hash(std::size_t old, std::size_t mod)
        {
            // assert(is_power_of_two(mod));
            return (((old << 2) + old) + 1) & (mod - 1);
        }

        constexpr std::size_t useable_fraction(std::size_t n)
        {
            return (n << 1) / 3;
        }

        template <typename T, bool StoreCache>
        struct storage_impl
        {
            T m_value;

            constexpr auto& value() noexcept 
            { return m_value; }

            constexpr auto& value() const noexcept
            { return m_value; }

            constexpr storage_impl(const T& x) : m_value { x } { }
            constexpr storage_impl(T&& x) : m_value { std::move(x) } { }

        };

        template <typename T>
        struct storage_impl<T, true> : storage_impl<T, false>
        {
            std::size_t m_hash_code;

            template <typename... Args>
            constexpr storage_impl(std::size_t hash_code, Args&&... args)
                : storage_impl<T, false>{ args... }, m_hash_code{ hash_code } { }
        };

    } // namespace detail


    template <typename T, 
        typename HashFunction, 
        typename KeyEqual, 
        typename Allocator, 
        typename Config, 
        bool StoreHashCode,
        bool Duplicate = true,
        std::size_t MinSize = 8, 
        std::size_t PerturbShift = 5>
    class hash_table_impl : public Config
    {

        static_assert(Duplicate, "Don't support multi-key");

        using storage_type = detail::storage_impl<T, StoreHashCode>;
        using entry_type = std::variant<std::monostate, storage_type>;
        using vector_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<entry_type>;
        using self_type = hash_table_impl;

        template <bool Const>
        struct hash_iterator
        {

            using link_type = std::conditional_t<
                Const, 
                const hash_table_impl*,
                hash_table_impl*
            >;

            using value_type = std::conditional_t<Const, const T, T>;
            using reference = value_type&;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::bidirectional_iterator_tag;


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
                const auto sz = m_c->m_entries.size();
                m_idx ++;
                for(; m_idx < sz && m_c->check_item_is_empty(m_idx); m_idx++);
                return *this;
            } 

            constexpr hash_iterator operator++(int) noexcept
            {
                auto old = *this;
                ++ *this;
                return old;
            }

            constexpr hash_iterator& operator--() noexcept
            {
                m_idx --;
                for(; m_idx != 0 && m_c->check_item_is_empty(m_idx - 1); m_idx--);
                return *this;
            } 

            constexpr hash_iterator operator--(int) noexcept
            {
                auto old = *this;
                -- *this;
                return old;
            }

            constexpr bool operator==(const hash_iterator& rhs) const noexcept
            {
                assert(m_c == rhs.m_c);
                return m_idx == rhs.m_idx; // simply check idx
            }

            constexpr reference operator*() const noexcept 
            { return std::get<1>(m_c->m_entries[m_idx]).value(); }
            
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

        hash_table_impl() noexcept
            : m_size{ 0 }, m_hash{ }, m_ke{ }  
        {
            m_entries.resize(MinSize);
        }

        std::size_t size() const noexcept
        { return m_size; }

        bool empty() const noexcept
        { return size() == 0; }

        void clear() noexcept
        {
            m_size = 0;
            m_entries.clear();
            m_entries.resize(MinSize);
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
            return exist ? iterator(this, slot) : end();
        }

        template <typename K> 
        auto& operator[](K&& k) requires (config == config_type::map)
        { return insert(std::make_pair((K&&) k, typename Config::mapped_type())).first->second; }	

        template <typename K> const_iterator find(const K& x) const noexcept 
        { return const_cast<self_type&>(*this).find(x); }

        template <typename K> bool contains(const K& x) const noexcept 
        { return find(x) != end(); } 

        iterator erase(const_iterator pos) 
        { return remove_item_by_iterator(pos); }

        iterator erase(iterator pos) 
        { return remove_item_by_iterator(pos); }

        template <typename K> size_type erase(const K& x) 
        { return remove_item(x); }


        iterator begin() noexcept 
        {
            std::size_t idx = 0;
            for (; idx < m_entries.size() && check_item_is_empty(idx); idx++);
            // std::cout << idx << '\n';
            return { this, idx };
        }
        
        iterator end() noexcept 
        { return { this, m_entries.size() }; }

        const_iterator begin() const noexcept 
        { return const_cast<self_type&>(*this).begin(); }

        const_iterator end() const noexcept 
        { return const_cast<self_type&>(*this).end(); }

        const_iterator cbegin() const noexcept 
        { return begin(); }

        const_iterator cend() const noexcept 
        { return end(); }

        reverse_iterator rbegin() noexcept 
        { return std::make_reverse_iterator(end()); } 

        reverse_iterator rend() noexcept 
        { return std::make_reverse_iterator(begin()); } 

        const_reverse_iterator rbegin() const noexcept 
        { return std::make_reverse_iterator(end()); } 

        const_reverse_iterator rend() const noexcept 
        { return std::make_reverse_iterator(begin()); } 
        
        const_reverse_iterator rcbegin() const noexcept 
        { return rbegin(); } 

        const_reverse_iterator rcend() const noexcept 
        { return rend(); } 
    
    	void swap(hash_table_impl& rhs) 
        noexcept(std::conjunction_v<std::is_nothrow_swappable<hasher>, std::is_nothrow_swappable<key_equal>>)
        {
            std::swap(m_size, rhs.m_size);
            m_entries.swap(rhs.m_entries);
            std::swap(m_hash, rhs.m_hash);
            std::swap(m_ke, rhs.m_ke);
        }

    private:
        std::vector<entry_type, vector_allocator_type> m_entries;
        std::size_t m_size; 
        [[no_unique_address]] hasher m_hash;
        [[no_unique_address]] key_equal m_ke;

        template <typename U>
        std::pair<std::size_t, bool> insert_unique(U&& x)
        {

            try_expand();

            std::size_t hash = m_hash(Config::get_key(x));
            auto [slot, exits] = find_entry(x, hash);

            if (!exits)
            {
                // try insert
                if constexpr (StoreHashCode)
                {
                    m_entries[slot].template emplace<1>((U&&)x, hash);
                }
                else
                {
                    m_entries[slot].template emplace<1>((U&&) x);
                }
                m_size++;
            }
            
            // x exist 
            return { slot, exits };
        }

        bool check_item_is_empty(std::size_t idx) const noexcept
        { return m_entries[idx].index() == 0; }

        template <typename K> 
        std::pair<std::size_t, bool> find_entry(const K& x, std::size_t hash) const noexcept // assume hasher is noexcept
        {
            std::size_t mask = m_entries.size() - 1;
            std::size_t slot = hash & mask; // keep slot less than table size
            std::size_t perturb = hash;
            bool exist;
            while (1)
            {
                if (check_item_is_empty(slot))
                {
                    exist = false;
                    break;
                }

                // if (m_ke(Config::get_key(std::get<1>(m_entries[slot])), Config::get_key(x)))
                if (Config::get_key(std::get<1>(m_entries[slot]).value()) == Config::get_key(x))
                {
                    exist = true;
                    break;
                }

                slot = ((5 * slot) + 1 + perturb) & mask;
                perturb >>= PerturbShift;
            }
            return { slot, exist };
        }

        std::size_t find_entry_only_with_hash_code(std::size_t hash) const noexcept // assume hasher is noexcept
        {
            std::size_t mask = m_entries.size() - 1;
            std::size_t slot = hash & mask; // keep slot less than table size
            std::size_t perturb = hash;
            while (!check_item_is_empty(slot))
            {
                slot = ((5 * slot) + 1 + perturb) & mask;
                perturb >>= PerturbShift;
            }
            return slot;
        }

        void try_expand()
        {
            if (m_size < detail::useable_fraction(m_entries.size()))
            {
                return; // more than 2/3 will expand
            }

            std::vector<entry_type, vector_allocator_type> old_entries(growth_rate());
            old_entries.swap(m_entries);

            for (auto& entry : old_entries)
            {
                // if there is not empty
                if (entry.index() != 0)
                {
                    std::size_t hash_code;
                    if constexpr (StoreHashCode)
                    {
                        hash_code = std::get<1>(entry).m_hash_code;
                    }
                    else
                    {
                        hash_code = m_hash(Config::get_key(std::get<1>(entry).value()));
                    }
                    
                    auto slot = find_entry_only_with_hash_code(hash_code);

                    m_entries[slot] = std::move(entry);
                }
            }
            return;
        }

        std::size_t growth_rate() const noexcept
        {
            std::size_t factor = m_entries.size() < 256 ? 4 : 2;
            return m_entries.size() * factor;
        }

        template <typename K>
        size_type remove_item(const K& x)
        {
            const auto hash = m_hash(x);
            auto [slot, exist] = find_entry(x, hash);
            if (exist)
            {
                m_entries[slot].template emplace<0>(std::monostate{});
                m_size--;
                return 1;
            }
            return 0;
        }

        template <typename I>
        iterator remove_item_by_iterator(I iter)
        {
            auto slot = iter.m_idx;
            m_entries[slot].emplace<0>(std::monostate{});
            m_size--;
            
            for (std::size_t i = slot + 1; i < m_entries.size(); ++i)
                if (!check_item_is_empty(i))
                    return { this, i };
            return { this, m_entries.size() };

        }

        template <typename K>
        std::size_t find_item(const K& x) const
        {
            auto hash = m_hash(x);
            auto [slot, exist] = find_entry(x, hash);
            return exist ? slot : m_entries.size();
        }

    };

    template <typename T, 
        typename HashFunction = std::hash<T>, 
        typename KeyEqual = std::equal_to<void>, 
        typename Allocator = std::allocator<T>>
    using hash_table = hash_table_impl<
        T, 
        HashFunction, 
        KeyEqual, 
        Allocator, 
        hash_set_config<T, HashFunction, KeyEqual, Allocator>, 
        false>;

    template <typename K, typename V, 
        typename HashFunction = std::hash<K>, 
        typename KeyEqual = std::equal_to<void>, 
        typename Allocator = std::pair<const K, V>>
    using hash_map = hash_table_impl<
        std::pair<K, V>, 
        HashFunction, 
        KeyEqual, 
        Allocator, 
        hash_map_config<K, V, HashFunction, KeyEqual, Allocator>, 
        false>;


}

