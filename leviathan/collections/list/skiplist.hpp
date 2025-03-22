#pragma once

#include <random>

#include "../common.hpp"
#include "../associative_container_interface.hpp"

namespace leviathan::collections
{
    
/**
 * @brief A skiplist implementation.
 * 
 * @param KeyValue Extractor extract key from value. identity<T> for set and select1st<K, V> for map.
 * @param Compare
 * @param Allocator
 * @param UniqueKey True for set/map and False for multiset/multimap.
 * @param RandomNumberGenerator Random generator to generate random numbers.
 * @param MaxLevel Max level of node.
 * @param Ratio Reciprocal of probability.
*/
template <typename KeyValue, 
    typename Compare, 
    typename Allocator, 
    bool Unique, 
    typename RandomNumberGenerator = std::random_device,
    int MaxLevel = 24, 
    int Ratio = 4>
class skiplist : public associative_container_insertion_interface,
                 public associative_container_lookup_interface<Unique>,
                 public reversible_container_interface
{
    static_assert(Unique, "Not support multi-key now");
    static_assert(Ratio > 1);

    using node_allocator = std::allocator_traits<Allocator>::template rebind_alloc<char>;
    using node_alloc_traits = std::allocator_traits<node_allocator>;

    [[deprecated("use get_level instead")]]	
    static int get_level1()
    {
        // return get_level_debug(); // debug
        static std::random_device rd;
        constexpr auto p = rd.max() / Ratio;
        int level = 1;
        for (; rd() < p; ++level);
        return std::min(MaxLevel, level);
    }

    static int get_level()
    {
        static RandomNumberGenerator rd;
        constexpr typename RandomNumberGenerator::result_type p = std::lerp(
            RandomNumberGenerator::min(), 
            RandomNumberGenerator::max(), 
            1.0 / Ratio);

        int level = 1;
        for (; rd() < p; ++level);
        return std::min(MaxLevel, level);
    }

public:

    using key_value = KeyValue;
    using key_type = typename key_value::key_type;
    using value_type = typename key_value::value_type;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using value_compare = Compare;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;
    using size_type = size_t;
    using allocator_type = Allocator;

private:

    // https://en.wikipedia.org/wiki/Flexible_array_member
    // layout may be a better choice to replace flexible array which is forbidden in ISO C++.
    struct skiplist_node
    {
        // Value field
        alignas(value_type) unsigned char m_raw[sizeof(value_type)];

        // Size of next
        int m_cnt;

        // Double recycle link list
        skiplist_node* m_prev;

        // Flexible array is forbidden in ISO C++.
        skiplist_node* m_next[];

        value_type *value_ptr()
        {
            return reinterpret_cast<value_type*>(m_raw);
        }

        const value_type *value_ptr() const
        {
            return reinterpret_cast<const value_type*>(m_raw);
        }

        void reset(skiplist_node* prev, skiplist_node* next)
        {
            m_prev = prev; 
            std::fill(m_next, m_next + m_cnt, next); 
        }

        static skiplist_node* allocate_node(node_allocator& alloc, int count)
        {
            const auto size = sizeof(skiplist_node) + count * sizeof(skiplist_node*);
            // For some fancy pointer such as FancyPtr<T>, the allocator may return 
            // FancyPtr<char>, so we cast the pointer to void* firstly.
            skiplist_node* node = reinterpret_cast<skiplist_node*>(
                static_cast<char*>(node_alloc_traits::allocate(alloc, size))
            );
            node->m_cnt = count;
            return node;
        }

        static void deallocate(node_allocator& alloc, skiplist_node* node)
        {
            const auto size = sizeof(skiplist_node) + node->m_cnt * sizeof(skiplist_node*);
            node_alloc_traits::deallocate(alloc, reinterpret_cast<char*>(node), size);
        } 

    };

    template <typename U> 
    using key_arg_t = detail::key_arg<detail::transparent<Compare>, U, key_type>;

    struct skiplist_iterator : bidirectional_iterator_interface
    {
        using link_type = skiplist_node*;
        using value_type = value_type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using reference = std::conditional_t<std::is_same_v<value_type, key_type>, const value_type&, value_type&>;

        link_type m_ptr;

        skiplist_iterator() = default;
        skiplist_iterator(const skiplist_iterator&) = default;
        skiplist_iterator(link_type ptr) : m_ptr(ptr) { }

        skiplist_iterator& operator++()
        {
            m_ptr = m_ptr->m_next[0];
            return *this;
        }

        using bidirectional_iterator_interface::operator++;

        skiplist_iterator& operator--()
        {
            m_ptr = m_ptr->m_prev;
            return *this;
        }

        using bidirectional_iterator_interface::operator--;

        reference operator*() const
        {
            return *(m_ptr->value_ptr());
        }

        bool operator==(this skiplist_iterator lhs, skiplist_iterator rhs)
        {
            return lhs.m_ptr == rhs.m_ptr;
        }

        constexpr skiplist_iterator skip(difference_type i) const
        {
            return skiplist_iterator(m_ptr->m_next[i]);
        }

        constexpr skiplist_iterator &skip_to(difference_type i)
        {
            return *this = skip(i);
        }

        constexpr void set_next(difference_type i, skiplist_iterator p)
        {
            m_ptr->m_next[i] = p.m_ptr;
        }

        constexpr void set_prev(skiplist_iterator p)
        {
            m_ptr->m_prev = p.m_ptr;
        }

        constexpr auto level() const
        {
            return m_ptr->m_cnt;
        }
    };

    static constexpr bool IsNothrowMoveConstruct = 
                std::is_nothrow_move_constructible_v<Compare> 
                && typename node_alloc_traits::is_always_equal();

    static constexpr bool IsNothrowMoveAssign = 
                std::is_nothrow_move_assignable_v<Compare> 
                && typename node_alloc_traits::is_always_equal();

    static constexpr bool IsNothrowSwap = 
                std::is_nothrow_swappable_v<Compare> 
                && typename node_alloc_traits::is_always_equal();

public:

    using iterator = skiplist_iterator;
    using const_iterator = std::const_iterator<iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    skiplist(const Compare& compare, const Allocator& allocator)
        : m_cmp(compare), m_alloc(allocator), m_size(0), m_level(1) 
    {
        m_header = skiplist_node::allocate_node(m_alloc, MaxLevel);
        reset_header();
    }

    skiplist() : skiplist(Compare(), Allocator()) { }

    // TODO
    skiplist(const skiplist&) = delete;
    skiplist(const skiplist&, const allocator_type&) = delete;
    skiplist(skiplist&&) = delete;
    skiplist(skiplist&&, const allocator_type&) = delete;
    skiplist& operator=(const skiplist&) = delete;
    skiplist& operator=(skiplist&&) = delete;

    ~skiplist()
    {
        clear();
    }

    iterator begin()
    {
        return iterator(m_header->m_next[0]);
    }

    const_iterator begin() const
    {
        return iterator(m_header->m_next[0]);
    }

    iterator end()
    {
        return iterator(m_header);
    }

    const_iterator end() const
    {
        return iterator(m_header);
    }

    size_type size() const
    {
        return m_size;
    }

    bool empty() const
    {
        return size() == 0;
    }

    allocator_type get_allocator() const
    {
        return allocator_type(m_alloc);
    }

    size_type max_size() const
    {
        return node_alloc_traits::max_size(m_alloc);
    }

    // Observers
    key_compare key_comp() const
    {
        return m_cmp;
    }

    value_compare value_comp() const
    {
        return m_cmp;
    }

    int current_level() const
    {
        return m_level;
    }

    template <typename K = key_type>
    iterator lower_bound(const key_arg_t<K>& x)
    {
        auto [node, exist] = find_node(x);
        return exist ? iterator(node) : std::next(node);
    }

    template <typename K = key_type>
    const_iterator lower_bound(const key_arg_t<K> &x) const
    {
        return const_cast<skiplist&>(*this).lower_bound(x);
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        if constexpr (detail::emplace_helper<value_type, Args...>::value)
        {
            return insert_unique((Args&&) args...);
        }
        else
        {
            value_handle<value_type, allocator_type> handle(m_alloc, (Args&&) args...);
            return insert_unique(*handle);
        }
    }

    // void show() const
    // {
    //     constexpr auto width = 5;
    //     std::cout << "Header: " << header()->m_cnt << " Level: " << current_level() << '\n';
    //     for (auto iter = begin(); iter != end(); ++iter)
    //     {
    //         std::string ss = std::to_string(*iter);
    //         if (ss.size() < width)
    //             ss.append(width - ss.size(), ' ');
    //         std::cout << "Value = " << ss << " (" << iter.m_ptr->m_cnt << ")";
    //         for (int i = 0; i < m_level; ++i)
    //         {
    //             std::string s = std::to_string(*iter);
    //             if (s.size() < width)
    //                 s.append(width - s.size(), ' ');
    //             if (i < iter.m_ptr->m_cnt)
    //                 std::cout << " -> " << s.substr(0, width);
    //             else if (header()->m_next[i] != iter.m_ptr)
    //                 std::cout << "    " << "|    ";
    //         }
    //         std::cout << "\n";
    //     }
    //     std::cout << "\n"; 
    // }

    void clear()
    {
        reset();
    }

    iterator erase(const_iterator pos) 
        requires (!std::same_as<iterator, const_iterator>)
    {
        return erase(pos.base());
    }

    iterator erase(iterator pos)
    {
        auto ret = std::next(pos);
        erase_node_by_value(*pos);
        return ret;
    }

    iterator erase(iterator first, iterator last)
    {
        if (first == begin() && last == end()) 
        {
            clear();
        }
        else
        {
            for (; first != last; first = erase(first));
        }
        return last;
    }

    iterator erase(const_iterator first, const_iterator last)
        requires(!std::same_as<iterator, const_iterator>)
    {
        return erase(first.base(), last.base());
    }

    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2077r3.html
    template <typename K> requires (detail::transparent<Compare>)
    size_type erase(K&& x) 
    {
        auto old_size = size();
        erase_node_by_value(x);
        return old_size - size();
    }

    size_type erase(const key_type& x)
    {
        auto old_size = size();
        erase_node_by_value(x);
        return old_size - size();
    }

private:

    void reset()
    {
        auto node = m_header->m_next[0];
        while (node != m_header)
        {
            auto next = node->m_next[0];
            drop_node(node);
            node = next;
        }
        m_level = 1;
        m_size = 0;
        reset_header();
    }

    template <typename U>
    std::pair<iterator, bool> insert_unique(U&& val)
    {
        std::array<iterator, MaxLevel> prev;
        prev.fill(end());
        auto [cur, exist] = find_node_with_prev(KeyValue()(val), prev);

        if (exist)
            return { cur, false };

        // insert a new node
        auto new_node = create_node((U&&) val);
        new_node->reset(cur.m_ptr, header());
        cur.skip(0).set_prev(new_node);
        iterator worker { new_node };
        iterator head { header() };
        
        // update header
        const int level = new_node->m_cnt;
        for (int i = 0; i < level; ++i)
        {
            if (i >= m_level)
            {
                head.set_next(i, worker);
            }
            else
            {
                worker.set_next(i, prev[i].skip(i));
                prev[i].set_next(i, worker);
            }
        }
        m_level = std::max(m_level, level);
        ++m_size;
        return { worker, true };
    }

    skiplist_node* header()
    { return m_header; }

    const skiplist_node* header() const 
    { return m_header; }

    // return target node if succeed otherwise prev of target value 
    template <typename K>
    std::pair<iterator, bool> find_node(const K& val) 
    {
        auto cur = iterator(header());
        auto sent = end();
        for (int i = m_level; i >= 0; --i)
        {
            for (; cur.skip(i) != sent && m_cmp(KeyValue()(*cur.skip(i)), val); cur.skip_to(i));
            auto next = cur.skip(i);
            if (next != sent && !m_cmp(val, KeyValue()(*next)))
            {
                return { next, true };
            }
        }
        return { cur, false };
    }

    // return target node if succeed otherwise prev position of target node for inserting
    template <typename K>
    std::pair<iterator, bool> find_node_with_prev(const K& val, std::array<iterator, MaxLevel>& prev) 
    {
        iterator cur = header();
        iterator sent = end();
        bool exits = false;
        for (int i = m_level - 1; i >= 0; --i)
        {
            for (; cur.skip(i) != sent && m_cmp(KeyValue()(*cur.skip(i)), val); cur.skip_to(i));
            auto next = cur.skip(i);
            // cur is prev of position, so val <= *next
            if (next != end() && !m_cmp(val, KeyValue()(*next)))
            {
                exits = true; // find it and do nothing
            }
            prev[i] = cur;
        }
        // cur is prev of node
        return { cur, exits };
    }

    template <typename K>
    void erase_node_by_value(const K& val)
    {
        std::array<iterator, MaxLevel> prev;
        prev.fill(end());
        auto [cur, exist] = find_node_with_prev(val, prev);

        if (!exist)
            return;

        auto deleted_pos = cur.skip(0);
        deleted_pos.skip(0).set_prev(cur);
        for (int i = 0; i < deleted_pos.level(); ++i)
            prev[i].set_next(i, deleted_pos.skip(i));

        int new_level;
        iterator sent = end();
        iterator head = iterator{ header() };
        for (new_level = m_level - 1; new_level >= 0 && head.skip(new_level) == sent; --new_level);
        m_level = new_level + 1;

        drop_node(deleted_pos.m_ptr);
        --m_size;
        return;
    }

    skiplist_node* get_node(int count)
    {
        skiplist_node* node = skiplist_node::allocate_node(m_alloc, count);
        return node;
    }

    template <typename... Args>
    void construct_node(skiplist_node* node, Args&&... args)
    {
        try
        {
            node_alloc_traits::construct(m_alloc, node->value_ptr(), (Args&&) args...);
        }
        catch (...)
        {
            dealloc_node(node);
            throw;
        }
    }

    void put_node(skiplist_node *p)
    {
        skiplist_node::deallocate(m_alloc, p);
    }

    template <typename... Args>
    skiplist_node* create_node(Args&&... args)
    {
        skiplist_node* tmp = get_node(get_level());
        construct_node(tmp, (Args&&) args...);
        return tmp;
    }

    void destroy_node(skiplist_node *p)
    {
        node_alloc_traits::destroy(m_alloc, p->value_ptr());
    }

    void drop_node(skiplist_node* p)
    {
        destroy_node(p);
        put_node(p);
    }

    void reset_header()
    {
        assert(header()->m_cnt == MaxLevel);
        header()->reset(header(), header());
    }

    [[no_unique_address]] Compare m_cmp;
    [[no_unique_address]] node_allocator m_alloc;
    size_type m_size;
    int m_level;
    skiplist_node* m_header;
};


} // namespace leviathan::collections

