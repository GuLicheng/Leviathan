/**
 *  This is a hash table with quadratic
 */

#ifndef __HASH_TABLE_HPP__
#define __HASH_TABLE_HPP__

#include <iostream>
#include <memory>
#include <functional>

namespace leviathan
{

    template <typename Key, typename Value, typename HashFunction = std::hash<Key>, typename KetEqual = std::equal_to<>, typename Allocator = std::allocator<std::pair<Key, Value>>>
    struct hash_map_config
    {
        using key_type = Key;
        using value_type = std::pair<Key, Value>;
        using hasher = HashFunction;
        using allocator_type = Allocator;
        using key_equal = KetEqual;

        // Lhs is value_type, and rhs is key_type
        bool compare(const Conpare& cmp, const Lhs& lhs, const Rhs& rhs) noexcept
        { return cmp(lhs.first, rhs); }
    };

    template <typename Key, typename HashFunction = std::hash<Key>, typename KetEqual = std::equal_to<>, typename Allocator = std::allocator<Key>>
    struct hash_set_config
    {
        using key_type = Key;
        using value_type = Key;
        using hasher = HashFunction;
        using allocator_type = Allocator;
        using key_equal = KetEqual;
        template <typename Compare, typename Lhs, typename Rhs>
        bool compare(const Conpare& cmp, const Lhs& lhs, const Rhs& rhs) noexcept
        { return cmp(lhs, rhs); }
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

        constexpr static int DefaultSize = 17; // for init table
        enum class state : unsigned int { active, empty, deleted };

        std::size_t size() const noexcept
        { return this->m_size; }
        
        std::size_t empty() const noexcept
        { return this->m_size == 0; }

        struct entry
        {
            value_type m_data;
            state m_state;
            entry(const value_type& x = value_type{ }) 
                : m_data{ x }, m_state{ state::empty } 
            {
            }
            entry(value_type&& x) 
                : m_data{ std::move(x) }, m_state{ state::empty }
            {
            }
        };
        using entry_allocator_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<value_type>;
        

    private:
        [[no_unique_address]] hasher m_hash;
        [[no_unique_address]] key_equal m_key_equal;
        [[no_unique_address]] entry_allocator_type m_alloc;
        std::size_t m_size;
        std::vector<entry> m_table;

        static bool is_prime(std::size_t x) noexcept
        {
            // use table to optimize
            for (std::size_t i = 2; i * i <= x; ++i)
                if (x % i == 0) return false;
            return true;
        }

        std::size_t get_index(const key_type& x) const noexcept // assert hasher and key_compare is exception-safe
        {
            std::size_t offset = 1;
            std::size_t index = this->m_hash(x) % this->m_table.size();
            // conclusion
            while (this->m_table[index].m_state != state::empty 
                && !Configuration::compare(this->m_key_equal, this->m_table[index].m_data, x))
            {
                index += offset;
                offset += 2;
                if (index >= this->m_table.size()) index -= this->m_table.size();
            }
            return index;
        }

        bool is_active(std::size_t index) const noexcept
        { return this->m_table[index].m_state == state::active; }

        template <typename T>
        std::pair<entry*, bool> insert_unique(T&& x)
        {
            auto index = get_index(x);
            if (is_active(index))
                return { &this->m_table[index], false }; // duplicate
            this->m_table[index].m_data = std::forward<T>(x); 
            this->m_table[index].m_state = state::active;
            if (++this->m_size > this->m_table() >> 1) 
                index = rehash(index);
            return { &this->m_table[index], true }; // 
        }

        std::size_t rehash(std::size_t index)  
        {
            std::vector<entry> new_table;
            new_table.reserve(next_size(this->m_table.size() << 1));
            // some issues ...
        }

        static std::size_t next_size(std::size_t x) noexcept
        {
            if (!(x & 1)) ++x;
            for (; !is_prime(x); x += 2);
            return x;
        }

    };
    
}



#endif