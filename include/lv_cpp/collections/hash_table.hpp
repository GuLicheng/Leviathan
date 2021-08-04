/**
 *  This is a hash table with quadratic
 */

#ifndef __HASH_TABLE_HPP__
#define __HASH_TABLE_HPP__

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
        enum class state : unsigned int { empty = 0, active, deleted };

    private:
        using entry_type = value_type;
        using entry_allocator_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<value_type>;
        using table_type = std::vector<entry_type, entry_allocator_type>;

        // TODO: implement hash_iterator
        template <bool> 
        struct hash_iterator;
    
    public:
        using const_iterator = hash_iterator<true>;
        using iterator = hash_iterator<true>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        hash_table() noexcept
            : m_hash{ }, m_key_equal{ }, m_size{ }
        {
            init_table();
        }

        hash_table(const hash_table&) = default;
        hash_table(hash_table&&) noexcept(noexcept(true)) = default;  // FIXME: noexcept
        hash_table& operator=(const hash_table&) = default;
        hash_table& operator=(hash_table&&) noexcept(noexcept(true)) = default; // // FIXME: noexcept

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

        void insert(const value_type& x)
        {
            insert_unique(x);
        }

        void insert(value_type&& x)
        {
            insert_unique(std::move(x));
        }

        void show() const
        {
            for (std::size_t i = 0; i < this->m_state.size(); ++i)
            {
                if (this->m_state[i] == state::active)
                    std::cout << this->m_table[i] << ' ';
            }
        }



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

            // debug 
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
            // vector with any allocator should satisfied copy or move semantics 
            std::vector old = std::move(this->m_table);
            std::vector old_state = std::move(this->m_state);

            // reset table
            const auto new_capacity = next_size(old.capacity());
            this->m_table.reserve(new_capacity); // realloc memory

            // reset states
            this->m_state.resize(new_capacity);  // default is static_cast<state>(0)

            // reset size
            this->m_size = 0;

            for (std::size_t i = 0; i < old_state.size(); ++i)
            {
                if (old_state[i] == state::active) 
                    insert(std::move(old[i]));
            }
        }

        std::size_t next_size(std::size_t sz) const noexcept
        {
            constexpr auto max_size = sizeof(prime_table) / sizeof(prime_table[0]);
            auto iter = std::find(prime_table, prime_table + max_size, sz);
            assert(iter != prime_table + max_size); // increase size if necessary
            return *(++iter);
        }
    };

    template <typename Key, typename HashFunction = std::hash<Key>, typename KeyEqual = std::equal_to<>, typename Allocator = std::allocator<Key>>
    class hash_set : public hash_table<hash_set_config<Key, HashFunction, KeyEqual, Allocator>> { };

    template <typename Key, typename Value, typename HashFunction = std::hash<Key>, typename KeyEqual = std::equal_to<>, typename Allocator = std::allocator<Key>>
    class hash_map : public hash_table<hash_map_config<Key, Value, HashFunction, KeyEqual, Allocator>> { };


}



#endif