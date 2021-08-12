/**
 *  This is a hash table with quadratic
 */

#ifndef __HASH_TABLE_HPP__
#define __HASH_TABLE_HPP__

#include <lv_cpp/meta/meta.hpp>

#include <iostream>
#include <memory>
#include <functional>
#include <algorithm>
#include <type_traits>
#include <assert.h>

namespace leviathan
{

    template <typename Key, typename Value, typename HashFunction = std::hash<Key>, typename KeyEqual = std::equal_to<>, typename Allocator = std::allocator<std::pair<Key, Value>>>
    struct hash_map_config
    {
        using key_type = Key;
        using value_type = std::pair<Key, Value>;
        using hasher = HashFunction;
        using allocator_type = Allocator;
        using key_equal = KeyEqual;

        // Lhs is value_type, and rhs is key_type
        template <typename Compare, typename Lhs, typename Rhs>
        constexpr static bool compare(const Compare& cmp, const Lhs& lhs, const Rhs& rhs) noexcept
        {
            return cmp(lhs.first, rhs);
        }

        template <typename T>
        constexpr static auto& key(const T& x) noexcept
        {
            return x.first;
        }

    };

    template <typename Key, typename HashFunction = std::hash<Key>, typename KeyEqual = std::equal_to<>, typename Allocator = std::allocator<Key>>
    struct hash_set_config
    {
        using key_type = Key;
        using value_type = Key;
        using hasher = HashFunction;
        using allocator_type = Allocator;
        using key_equal = KeyEqual;

        template <typename Compare, typename Lhs, typename Rhs>
        constexpr static bool compare(const Compare& cmp, const Lhs& lhs, const Rhs& rhs) noexcept
        {
            return cmp(lhs, rhs);
        }

        template <typename T>
        constexpr static auto& key(const T& x) noexcept
        {
            return x;
        }
    };


    template <typename Configuration>
    class hash_table
    {

    public:
        using value_type = typename Configuration::value_type;
        using key_type = typename Configuration::key_type;
        using allocator_type = typename Configuration::allocator_type;
        using hasher = typename Configuration::hasher;
        using key_equal = typename Configuration::key_equal;
        enum class state : unsigned char { empty = 0, active, deleted };

    private:
        using entry_type = value_type;
        using entry_allocator_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<value_type>;
        using table_type = std::vector<entry_type, entry_allocator_type>;

        template <bool> 
        struct hash_iterator;
    
    public:
        using const_iterator = hash_iterator<true>;
        using iterator = hash_iterator<false>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        hash_table() noexcept
            : m_hash{ }, m_key_equal{ }, m_size{ }
        {
            init_table();
        }

        hash_table(const hash_table& rhs)
             : m_hash{ rhs.m_hash }, m_key_equal{ rhs.m_key_equal }, m_size{ 0 }
        {
            const auto sz = rhs.m_state.size();
            this->m_state.resize(sz); // static_cast<state>(0) -> state::empty
            this->m_table.reserve(sz);
            assign_from(rhs.begin(), rhs.end());
            // for (std::size_t i = 0; i < rhs.sz; ++i)
            //     if (rhs.is_active(i))
            //         insert(rhs.m_table[i]);
        }


        hash_table& operator=(const hash_table& rhs) 
        {
            if (this != std::addressof(rhs))
            {
                clear();
                const auto sz = rhs.m_state.size();
                this->m_state.resize(sz); // static_cast<state>(0) -> state::empty
                this->m_table.reserve(sz);
                this->m_hash = rhs.m_hash;
                this->m_key_equal = rhs.m_key_equal;
                assign_from(rhs.begin(), rhs.end());
            }
            return *this;
        }

        hash_table(hash_table&&) noexcept(noexcept(true)) = default;  // FIXME: 
        hash_table& operator=(hash_table&&) noexcept(noexcept(true)) = default; // // FIXME:

        ~hash_table() noexcept
        {
            clear();
        }

        void clear() noexcept
        {
            // destory 
            destory_objects();
            // reset state
            this->m_size = 0;
            this->m_state.clear();
            this->m_table.clear();
            // keep table size is prime
            init_table();
        }

        std::size_t size() const noexcept
        {
            return this->m_size;
        }

        std::size_t empty() const noexcept
        {
            return this->m_size == 0;
        }

        std::pair<iterator, bool> insert(const value_type& x)
        {
            auto [entry, exist] = insert_unique(x);
            auto cur = std::distance(this->m_table.data(), entry);
            return { iterator(cur, this), !exist };
        }

        std::pair<iterator, bool> insert(value_type&& x)
        {
            auto [entry, exist] = insert_unique(x);
            auto cur = std::distance(this->m_table.data(), entry);
            return { iterator(cur, this), !exist };
        }

        iterator find(const key_type& x) 
        {
            auto entry = find_entry(x);
            auto cur = std::distance(this->m_table.data(), entry);
            return { cur, this };
        }

        const_iterator find(const key_type& x) const
        {
            return const_cast<hash_table*>(this)->find(x);
        }

        iterator erase(const key_type& x)
        {
            auto entry = erase_entry(x);
            auto cur = std::distance(this->m_table.data(), entry);
            return { cur, this };
        }

        void show() const
        {
            for (std::size_t i = 0; i < this->m_state.size(); ++i)
            {
                if (this->m_state[i] == state::active)
                    std::cout << this->m_table[i] << ' ';
            }
        }

        iterator begin() noexcept
        {
            auto cur = std::distance(
                this->m_state.begin(),
                std::find(this->m_state.begin(), this->m_state.end(), state::active)
            );
            return { cur, this }; 
        }

        iterator end() noexcept
        { return { static_cast<std::ptrdiff_t>(this->m_state.size()), this }; }

        const_iterator begin() const noexcept
        { return const_cast<hash_table*>(this)->begin(); }

        const_iterator end() const noexcept
        { return { static_cast<std::ptrdiff_t>(this->m_state.size()), this }; }

        const_iterator cbegin() const noexcept
        { return const_cast<hash_table*>(this)->begin(); }       

        const_iterator cend() const noexcept
        { return { static_cast<std::ptrdiff_t>(this->m_state.size()), this }; }

        reverse_iterator rbegin() noexcept
        { return std::make_reverse_iterator(end()); }

        reverse_iterator rend() noexcept
        { return std::make_reverse_iterator(begin()); }

        const_reverse_iterator rbegin() const noexcept
        { return std::make_reverse_iterator(end()); }

        const_reverse_iterator rend() const noexcept
        { return std::make_reverse_iterator(begin()); }

        const_reverse_iterator rcbegin() const noexcept
        { return std::make_reverse_iterator(end()); }

        const_reverse_iterator rcend() const noexcept
        { return std::make_reverse_iterator(begin()); }

    private:
        [[no_unique_address]] hasher m_hash;
        [[no_unique_address]] key_equal m_key_equal;
        std::size_t m_size;
        std::vector<entry_type, entry_allocator_type> m_table;
        std::vector<state> m_state;  // default allocator is OK

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

        void init_table() noexcept // assert our memory is enough
        {
            this->m_table.reserve(prime_table[0]);
            this->m_state.resize(prime_table[0]); // 0 for state::empty
        }

        std::size_t get_index(const key_type& x) const noexcept // assert hasher and key_compare is exception-safe
        {
            std::size_t offset = 1;
            const auto table_size = this->m_table.capacity();
            std::size_t index = this->m_hash(x) % table_size;
            // conclusion
            while (this->m_state[index] != state::empty
                && !Configuration::compare(this->m_key_equal, this->m_table[index], x))
            {
                index += offset;
                offset += 2;
                if (index >= table_size) index -= table_size;
            }
            return index;
        }

        bool is_active(std::size_t index) const noexcept
        {
            return this->m_state[index] == state::active;
        }

        bool is_empty(std::size_t index) const noexcept
        {
            return this->m_state[index] == state::empty;
        }

        bool is_deleted(std::size_t index) const noexcept
        {
            return this->m_state[index] == state::deleted;
        }

        template <typename T>
        std::pair<entry_type*, bool> insert_unique(T&& x)
        {
            auto index = get_index(Configuration::key(x));
            if (is_active(index))
                return { &this->m_table[index], false }; // duplicate

            // check capacity
            if (this->m_size + 1 > (this->m_table.capacity() >> 1))
                rehash();

            index = get_index(Configuration::key(x));
            if (is_deleted(index))
                std::destroy_at(&this->m_table[index]);

            // MSVC debug may not allowed
            std::construct_at(&this->m_table[index], std::forward<T>(x));
            this->m_state[index] = state::active;
            this->m_size++;
            return { &this->m_table[index], true };

        }

public:
        const entry_type* find_entry(const key_type& x) const 
        {
            return const_cast<hash_table*>(this)->find_entry(x);
        }

        entry_type* find_entry(const key_type& x) 
        {
            const auto index = get_index(x);
            // not active
            if (!is_active(index))
                return this->m_table.data() + this->m_table.capacity();
            return &(this->m_table[index]);
            // return nullptr;
        }

        entry_type* erase_entry(const key_type& x)
        {
            const auto index = get_index(x);
            // remove item
            if (is_active(index))
                this->m_state[index] = state::deleted;
            // get next item
            auto iter = std::find(this->m_state.begin() + index, this->m_state.end(), state::active);
            auto dist = std::distance(this->m_state.begin(), iter);
            return &(this->m_table[dist]);
        }
private:

        template <typename Iter, typename Sent>
        void assign_from(Iter iter, Sent sent)
        {
            while (iter != sent)
                insert(*iter);
        }

        void destory_objects()
        {
            const auto sz = this->m_state.size();
            for (std::size_t i = 0; i < sz; ++i)
            {
                if (this->m_state[i] != state::empty)
                {
                    std::destroy_at(&this->m_table[i]);
                    this->m_state[i] == state::empty;
                }
            }
        }

        void rehash()
        {
            std::vector<entry_type, entry_allocator_type> old;
            old.reserve(this->m_size);
            for (std::size_t i = 0; i < this->m_state.size(); ++i)
            {
                // move active elem and destory deleted elem
                switch (this->m_state[i])
                {
                    case state::active: old.emplace_back(std::move(this->m_table[i])); 
                    case state::deleted: std::destroy_at(&this->m_table[i]); break;
                    default: break;
                }
                this->m_state[i] = state::empty;
            }
            // reset table
            const auto new_capacity = next_size(this->m_table.capacity());
            this->m_table.reserve(new_capacity); // realloc memory

            // reset states
            this->m_state.resize(new_capacity);  // default is static_cast<state>(0)

            // reset size
            this->m_size = 0;

            // reinsert elem
            for (auto& val : old)
                insert(std::move(val));
        }

        static std::size_t next_size(std::size_t sz) noexcept
        {
            constexpr auto max_size = sizeof(prime_table) / sizeof(prime_table[0]);
            auto iter = std::find(prime_table, prime_table + max_size, sz);
            assert(iter != prime_table + max_size); // increase size if necessary
            return *(++iter);
        }
    };


    template <typename Configuration>
    template <bool Const>
    struct hash_table<Configuration>::hash_iterator
    {
        using link_container_type = meta::maybe_const_t<Const, hash_table<Configuration>*>;
        link_container_type m_container;
        std::ptrdiff_t m_cur;

        using value_type = typename Configuration::value_type;
        using reference = meta::maybe_const_t<Const, value_type&>;
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;

		constexpr hash_iterator() noexcept = default;
		constexpr hash_iterator(std::ptrdiff_t cur, link_container_type c) noexcept
			: m_cur{ cur }, m_container{ c } { }

		constexpr hash_iterator(const hash_iterator&) noexcept = default;

		template <bool IsConst, typename = std::enable_if_t<((Const == IsConst) || Const)>>
		constexpr hash_iterator(const hash_iterator<IsConst>& rhs) noexcept
			: m_cur{ rhs.m_cur }, m_container{ rhs.m_container } { }

		template <bool IsConst, typename = std::enable_if_t<((Const == IsConst) || Const)>>
		constexpr hash_iterator&
			operator=(const hash_iterator<IsConst>& rhs) noexcept
		{
			this->m_cur = rhs.m_cur;
			this->m_container = rhs.m_container;
		}

		template <bool IsConst>
		constexpr bool operator==(const hash_iterator<IsConst>& rhs) const noexcept
		{
			return this->m_container == rhs.m_container 
                && this->m_cur == rhs.m_cur;
		}

		template <bool IsConst>
		constexpr bool operator!=(const hash_iterator<IsConst>& rhs) const noexcept
		{
			return !this->operator==(rhs);
		}

		constexpr auto operator->() const noexcept
		{
			return &(this->operator*());
		}

		constexpr reference operator*() const noexcept
		{
            return this->m_container->m_table[this->m_cur];
		}

		constexpr hash_iterator& operator++()
		{
            // find next state::active
            auto& state = this->m_container->m_state;
            auto first = state.begin();
            auto last = state.end();
            auto now = first + this->m_cur;
            auto iter = std::find(now + 1, last, state::active);
            auto dist = std::distance(first, iter);
            this->m_cur = dist;
            return *this;
		}

		constexpr hash_iterator& operator--()
		{
            auto& state = this->m_container->m_state;
            auto first = state.begin();
            auto last = state.end();
            auto now = first + this->m_cur;

            auto iter = std::find(std::make_reverse_iterator(now), std::make_reverse_iterator(first), state::active);
            this->m_cur = std::distance(first, iter.base()) - 1;
            return *this;
		}

		constexpr hash_iterator operator++(int)
		{
			auto old = *this;
			++* this;
			return old;
		}

		constexpr hash_iterator operator--(int)
		{
			auto old = *this;
			--* this;
			return old;
		}

    };


    template <typename Key, typename HashFunction = std::hash<Key>, typename KeyEqual = std::equal_to<>, typename Allocator = std::allocator<Key>>
    class hash_set : public hash_table<hash_set_config<Key, HashFunction, KeyEqual, Allocator>> { };

    template <typename Key, typename Value, typename HashFunction = std::hash<Key>, typename KeyEqual = std::equal_to<>, typename Allocator = std::allocator<Key>>
    class hash_map : public hash_table<hash_map_config<Key, Value, HashFunction, KeyEqual, Allocator>> { };


}

#include <ranges>

static_assert(std::ranges::bidirectional_range<leviathan::hash_set<int>>);
static_assert(std::ranges::bidirectional_range<const leviathan::hash_set<int>>);

#endif