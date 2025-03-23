#pragma once

#include <leviathan/collections/common.hpp>
#include <leviathan/collections/container_interface.hpp>
#include <leviathan/utils/layout.hpp>
#include <leviathan/allocators/adaptor_allocator.hpp>
#include <leviathan/collections/list/skip_node.hpp>

#include <span>
#include <random>
#include <cstddef>

namespace leviathan::collections
{
    
/**
 * @brief A skiplist implementation.
 * 
 * @param KeyValue Extractor extract key from value. identity<T> for set and select1st<K, V> for map.
 * @param Compare
 * @param Allocator
 * @param UniqueKey True for set/map and False for multiset/multimap.
 * @param RandomEngine Random generator to generate random numbers.
 * @param SeedGenerator Seed generator for random engine.
 * @param MaxLevel Max level of node.
 * @param Ratio Reciprocal of probability.
*/
template <typename KeyValue, 
    typename Compare, 
    typename Allocator, 
    bool UniqueKey, 
    typename RandomEngine = std::mt19937,
    typename SeedGenerator = std::random_device,
    int MaxLevel = 24, 
    int Ratio = 4>
class skip_list : public container_interface
{
    static_assert(UniqueKey, "Not support multi-key now");
    static_assert(Ratio > 1);
    static_assert(std::is_unsigned_v<typename RandomEngine::result_type>);

    static constexpr auto pro = std::lerp(RandomEngine::min(), RandomEngine::max(), 1.0 / Ratio);

    inline static RandomEngine random = RandomEngine(SeedGenerator()());

    static int get_level()
    {
        int level = 1;
        for (; random() < pro; ++level);
        return std::min(MaxLevel, level);
    }

public:

    using key_value = KeyValue;
    using key_type = typename key_value::key_type;
    using value_type = typename key_value::value_type;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using value_compare = Compare;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;
    using size_type = size_t;
    using allocator_type = Allocator;
    using iterator = skip_iterator<key_value>;
    using const_iterator = std::const_iterator<iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:

    using list_node = skip_node<value_type>;
    using node_layout_type = typename list_node::node_layout_type;
    using node_allocator = adaptor_allocator<Allocator, std::byte>;

    template <typename U> 
    using key_arg_t = detail::key_arg<detail::transparent<Compare>, U, key_type>;

    static constexpr bool IsNothrowMoveConstruct = nothrow_move_constructible<Allocator, Compare>;
    static constexpr bool IsNothrowMoveAssign = nothrow_move_assignable<Allocator, Compare>;
    static constexpr bool IsNothrowSwap = nothrow_swappable<Allocator, Compare>;

public:

    template <typename Self>
    self_iter_t<Self> begin(this Self&& self) 
    {
        return iterator(as_non_const(self).header()->pointers()[1]);
    }

    template <typename Self>
    self_iter_t<Self> end(this Self&& self)
    {
        return iterator(as_non_const(self).header());
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
        using AllocTraits = std::allocator_traits<Allocator>;
        return AllocTraits::max_size(m_alloc);
    }

    // Modifiers
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
    
    size_type erase(const key_type& x)
    {
        auto old_size = size();
        erase_node_by_value(x);
        return old_size - size();
    }

    // Observers
    template <typename Self, typename K = key_type>
    self_iter_t<Self> lower_bound(this Self&& self, const key_arg_t<K>& x)
    {
        auto [it, exist] = as_non_const(self).find_node(x);
        return exist ? it : std::next(it);
    }

    template <typename Self, typename K = key_type>
    self_iter_t<Self> find(this Self&& self, const key_arg_t<K>& x)
    {
        // return self.lower_bound(x) == self.end() || self.m_cmp(x, KeyValue()(*self.lower_bound(x))) 
        //     ? self.end() : self.lower_bound(x);
    }

    key_compare key_comp() const
    {
        return m_cmp;
    }

    value_compare value_comp() const
    {
        return m_cmp;
    }

    int level() const
    {
        return m_level;
    }

    skip_list(const Compare& compare, const Allocator& allocator)
        : m_cmp(compare), m_alloc(allocator)
    {
        m_level = 1;
        m_size = 0;
        m_header = alloc_node(MaxLevel);
        reset_header();
    }

    skip_list() : skip_list(Compare(), Allocator()) { }

    ~skip_list() 
    {
        clear();
    }

    void clear()
    {
        reset();
    }

private:

    template <typename K>
    void erase_node_by_value(const K& val)
    {
        std::array<iterator, MaxLevel> prev;
        prev.fill(end());

        auto [cur, exist] = find_node_with_prev(val, prev);

        if (!exist)
        {
            return;
        }

        auto deleted_pos = cur.skip(0);
        deleted_pos.skip(0).set_prev(cur);

        for (int i = 0; i < deleted_pos.level(); ++i)
        {
            prev[i].set_next(i, deleted_pos.skip(i));
        }

        int new_level;
        iterator sent = end();
        iterator head = iterator{ header() };
        for (new_level = m_level - 1; new_level >= 0 && head.skip(new_level) == sent; --new_level);
        m_level = new_level + 1;

        drop_node(deleted_pos.link());
        --m_size;
        return;
    }

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

    template <typename U>
    std::pair<iterator, bool> insert_unique(U&& val)
    {
        std::array<iterator, MaxLevel> prev;
        prev.fill(end());

        auto [cur, exist] = find_node_with_prev(KeyValue()(val), prev);

        if (exist)
        {
            return { cur, false };
        }

        // insert a new node
        auto new_node = create_node((U&&) val);
        iterator worker { new_node };
        iterator head { header() };
        
        new_node->reset(cur.link(), header());
        cur.skip(0).set_prev(new_node);
        
        // update header
        const int level = new_node->count();

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

    // return target node if succeed otherwise prev position of target node for inserting
    template <typename K>
    std::pair<iterator, bool> find_node_with_prev(const K& val, std::span<iterator, MaxLevel> prev) 
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

    template <typename... Args>
    void construct_node(list_node* node, Args&&... args)
    {
        using AllocTraits = std::allocator_traits<Allocator>; 

        try
        {
            AllocTraits::construct(m_alloc, node->value_ptr(), (Args&&) args...);
        }
        catch (...)
        {
            dealloc_node(node);
            throw;
        }
    }

    void destroy_node(list_node* node)
    {
        using AllocTraits = std::allocator_traits<Allocator>;
        AllocTraits::destroy(m_alloc, node->value_ptr());
    }

    [[nodiscard]] list_node* alloc_node(int count)
    {
        auto size = node_layout_type(1, 1, 1 + count).template alloc_size();
        auto node = static_cast<list_node*>(
            static_cast<void*>(node_allocator::allocate(m_alloc, size))
        );

        node->count_ref() = count;
        return node;
    }

    void dealloc_node(list_node* node)
    {
        auto size = node->alloc_size();
        node_allocator::deallocate(m_alloc, node->self_ptr(), size);
    }

    void drop_node(list_node* node)
    {
        destroy_node(node);
        dealloc_node(node);
    }

    template <typename... Args>
    list_node* create_node(Args&&... args)
    {
        auto node = alloc_node(get_level());
        construct_node(node, (Args&&) args...);
        return node;
    }

    void reset_header()
    {
        assert(header()->count() == MaxLevel);
        header()->reset(m_header, m_header);
    }

    void reset()
    {
        auto node = header()->pointers()[0];

        while (node != header())
        {
            auto next = node->pointers()[0];
            drop_node(node);
            node = next;
        }

        m_level = 1;
        m_size = 0;
        reset_header();
    }

    list_node* header()
    { return m_header; }

    const list_node* header() const 
    { return m_header; }

    [[no_unique_address]] Compare m_cmp;
    [[no_unique_address]] Allocator m_alloc;
    int m_level;
    list_node* m_header;
    size_type m_size;

};

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
using skip_set = skip_list<identity<T>, Compare, Allocator, true>;


} // namespace leviathan::collections

