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

#include <type_traits>
#include <functional> // std::hash
#include <variant>
#include <bit>
#include <assert.h>



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

        /*
            hash policy:
            {
                ctor: (hash_code, table_size):
                    hash_code: std::hash(x) which may greater than table_size
                    table_size(): m_entries.size() - length(sentinel)
                static bool need_expand(double load_factor); -> check whether need to expand, factor = used_size / table_size
                static std::size_t next_capacity()(std::size_t table_size); -> return next capacity
            }
        */

        template <std::size_t Num = 1, std::size_t Den = 2>
        struct quadratic_policy
        {
        
            constexpr static double factor = (double) Num / Den;

            constexpr static bool need_expand(double load_factor) noexcept
            { return load_factor >= factor; }

            constexpr static std::size_t next_capacity(std::size_t table_size) noexcept
            {
                // FIXME call std::terminate if return sentinel
                return *std::ranges::find_if(prime_table, [=](const auto& x) {
                    return x > table_size;
                });
            }

            quadratic_policy(std::size_t hash_init, std::size_t mask) noexcept 
            {
                // assert(is_prime(mask));
                m_value = hash_init % mask;
                m_mask = mask;
                m_delta = 1;                
            }

            constexpr std::size_t operator()() noexcept
            {
                m_value += m_delta;
                m_delta += 2;
                m_value %= m_mask;
                return m_value;
            }

            constexpr std::size_t first() const noexcept
            { return m_value; }

        private:

            static inline std::size_t prime_table[] = {
                17, 29, 37, 59, 89, 127, 193, 293,
                433, 653, 977, 1459, 2203, 3307, 4967,
                7451, 11173, 16759, 25147, 37747, 56629,
                84947, 127423, 191137, 286711, 430081,
                645131, 967693, 1451539, 2177321,
                3265981, 4898981, 7348469, 11022709,
                16534069, 24801109, 37201663, 55802497, 83703749,
                125555621, 188333437, 282500161, 423750241, 635625377, 953438137
            };

            constexpr static bool is_prime(std::size_t x) noexcept
            {
                if (x < 2) return false;

                for (std::size_t i = 2; i * i <= x; ++i)
                    if (x % i == 0)
                        return false;
                return true;
            } 

            std::size_t m_value;
            std::size_t m_mask;
            std::size_t m_delta;
        };

        template <std::size_t Num = 2, std::size_t Den = 3, std::size_t PerturbShift = 5>
        struct python_dict
        {

            constexpr static double factor = (double) Num / Den;

            constexpr static bool need_expand(double load_factor) noexcept
            { return load_factor > factor; }

            constexpr static std::size_t next_capacity(std::size_t table_size) noexcept
            {
                assert(is_power_of_two(table_size));
                // std::size_t factor = table_size < 256 ? 4 : 2;
                return table_size * 2;
            }

            python_dict(std::size_t hash_init, std::size_t table_size) noexcept 
            {
                assert(is_power_of_two(table_size));
                m_mask = table_size - 1;
                m_perturb_shift = hash_init;
                m_value = hash_init & m_mask;
            }

            constexpr std::size_t operator()() noexcept
            {
                m_perturb_shift >>= PerturbShift;
                m_value = (5 * m_value + 1 + m_perturb_shift) & m_mask;
                return m_value;
            }

            constexpr std::size_t first() const noexcept
            { return m_value; }

        private:
            constexpr static bool is_power_of_two(std::size_t x) noexcept
            { return std::popcount(x) == 1; }
        
            std::size_t m_value;
            std::size_t m_perturb_shift;
            std::size_t m_mask;

        };

        template <typename T, bool Cache>
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

        template <typename T>
        struct cache_hash_code : std::true_type { };

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
    class hash_table_impl : public Config
    {

        static_assert(Duplicate, "Don't support multi-key");

        using storage_type = detail::storage_impl<T, StoreHashCode>;

        // simply use std::default_sentinel_t as our sentinel type
        // but define another sentinel and monostate may be better
        using entry_type = std::variant<std::monostate, storage_type, std::default_sentinel_t>;
        using vector_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<entry_type>;
        using self_type = hash_table_impl;

        template <bool Const>
        struct hash_iterator
        {

            using link_type = std::conditional_t< 
                Const,
                typename std::vector<entry_type, vector_allocator_type>::const_iterator,
                typename std::vector<entry_type, vector_allocator_type>::iterator>;

            using value_type = std::conditional_t<Const, const T, T>;
            using reference = value_type&;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            link_type m_cur;

            constexpr hash_iterator() noexcept = default;
            constexpr hash_iterator(const hash_iterator&) noexcept = default;
            constexpr hash_iterator(const hash_iterator<!Const>& rhs) noexcept requires (Const)
                : m_cur{ rhs.m_cur } { }

            constexpr hash_iterator(link_type cur) noexcept
                : m_cur{ cur } { }

            constexpr hash_iterator& operator++() noexcept
            {
                do ++m_cur; while (m_cur->index() == 0);  // skip empty and stop in active or sentinel
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
            { return std::get<1>(*m_cur).value(); }
            
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
            resize_and_make_sentinel(MinSize + 1);
        }

        std::size_t size() const noexcept
        { return m_size; }

        bool empty() const noexcept
        { return size() == 0; }

        void clear() noexcept
        {
            m_size = 0;
            m_entries.clear();
            resize_and_make_sentinel(MinSize + 1);
        }

        std::pair<iterator, bool> insert(const value_type& x) 
        {
            auto [slot, exist] = insert_unique(x);
            return { iterator(m_entries.begin() + slot), !exist };
        }

        std::pair<iterator, bool> insert(value_type&& x) 
        {
            auto [slot, exist] = insert_unique(std::move(x));
            return { iterator(m_entries.begin() + slot), !exist };
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
            return exist ? iterator(m_entries.begin() + slot) : end();
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
            return { m_entries.begin() + idx };
        }
        
        iterator end() noexcept 
        { return { m_entries.end() - 1 }; }

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

        // auto global_begin() { return m_entries.begin(); }
        // auto global_end() { return m_entries.end(); }
        void show_state() const {
            for (int i = 0; i < m_entries.size(); ++i)
                std::cout << "Index " << i << " State: " << m_entries[i].index() << '\n'; 
        }

    private:
        std::vector<entry_type, vector_allocator_type> m_entries;
        std::size_t m_size; 
        [[no_unique_address]] hasher m_hash;
        [[no_unique_address]] key_equal m_ke;

        
        void resize_and_make_sentinel(std::size_t sz)
        {
            m_entries.resize(sz);
            m_entries.back().template emplace<2>(std::default_sentinel);
        }

        template <typename U>
        std::pair<std::size_t, bool> insert_unique(U&& x)
        {

            rehash_and_grow_if_necessary();

            std::size_t hash = m_hash(Config::get_key(x));
            auto [slot, exits] = find_entry(Config::get_key(x), hash);

            if (!exits)
            {
                // try insert
                if constexpr (StoreHashCode)
                {
                    m_entries[slot].template emplace<1>(hash, (U&&)x);
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

        std::size_t table_size() const noexcept
        { return m_entries.size() - 1; }

        template <typename K> 
        std::pair<std::size_t, bool> find_entry(const K& x, std::size_t hash) const noexcept // assume hasher is noexcept
        {

            bool exist;

            HashPolicy hash_generator { hash, table_size() }; // hash_init, mask

            auto slot = hash_generator.first();

            while (1)
            {
                if (check_item_is_empty(slot))
                {
                    exist = false;
                    break;
                }

                if (m_ke(Config::get_key(std::get<1>(m_entries[slot]).value()), x))
                {
                    exist = true;
                    break;
                }
                slot = hash_generator();
            }
            return { slot, exist };
        }



        std::size_t find_entry_only_with_hash_code(std::size_t hash) const noexcept // assume hasher is noexcept
        {
            HashPolicy hash_generator { hash, table_size() }; // hash_init, mask
            auto slot = hash_generator.first();
            for (; !check_item_is_empty(slot); slot = hash_generator());
            return slot;
        }

        void rehash_and_grow_if_necessary()
        {

            double load_factor = (double)m_size / table_size();

            if (!HashPolicy::need_expand(load_factor))
            {
                return; 
            }


            const auto next_capacity = growth_rate() + 1;
            auto old_entries = std::move(m_entries);
            resize_and_make_sentinel(next_capacity);

            for (auto& entry : old_entries)
            {
                // if there is active
                if (entry.index() == 1)
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
        { return HashPolicy::next_capacity(table_size()); }

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
            auto cur = iter.m_cur;
            cur->template emplace<0>(std::monostate{});
            m_size--;
            return ++iter;
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
        hash_set_config<T, HashFunction, KeyEqual, Allocator>>;

    template <typename K, typename V, 
        typename HashFunction = std::hash<K>, 
        typename KeyEqual = std::equal_to<void>, 
        typename Allocator = std::allocator<std::pair<K, V>>>
    using hash_map = hash_table_impl<
        std::pair<K, V>, 
        HashFunction, 
        KeyEqual, 
        Allocator, 
        hash_map_config<K, V, HashFunction, KeyEqual, Allocator>>;

}
