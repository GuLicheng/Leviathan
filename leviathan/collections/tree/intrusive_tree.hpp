#pragma once

#include <leviathan/collections/core/node_interface.hpp>
#include <leviathan/collections/core/container_interface.hpp>
#include <leviathan/collections/core/tree_render.hpp>
#include <leviathan/collections/core/bs_tree_algorithm.hpp>
#include <leviathan/collections/common.hpp>

namespace cpp::collections::intrusive
{

template <std::meta::info MemberHook>
struct tree_hook_iterator
{
    using hook_type = typename [:type_of(MemberHook):];
    using value_type = typename [:parent_of(MemberHook):];
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    hook_type* current;

    constexpr tree_hook_iterator() = default;
    constexpr tree_hook_iterator(const tree_hook_iterator&) = default;
    constexpr tree_hook_iterator(hook_type* node) : current(node) { }
    
    constexpr reference operator*() const 
    { 
        return *from_hook<MemberHook>(current); 
    }

    constexpr pointer operator->() const 
    { 
        return from_hook<MemberHook>(current); 
    }

    constexpr tree_hook_iterator& operator++()
    {
        current = current->is_sentinel() ? current->lchild() : current->increment();
        return *this;
    }

    constexpr tree_hook_iterator operator++(int)
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }


    constexpr tree_hook_iterator& operator--()
    {
        current = current->is_sentinel() ? current->rchild() : current->decrement();
        return *this;
    }

    constexpr tree_hook_iterator operator--(int)
    {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    constexpr bool operator==(const tree_hook_iterator&) const = default;

};

template <
    std::meta::info MemberHook, 
    typename KeyOfValue = void, // Placeholder for key extraction function
    typename Compare = std::less<>, 
    bool UniqueKey = true
>
class tree : public iterable_interface
{

public:

    using hook_type = typename [:type_of(MemberHook):];
    using value_type = typename [:parent_of(MemberHook):];
    using pointer = value_type*;
    using reference = value_type&;
    using const_pointer = const value_type*;
    using const_reference = const value_type&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = tree_hook_iterator<MemberHook>;
    using const_iterator = std::const_iterator<iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr tree() : tree(Compare()) { }

    constexpr tree(const Compare& compare) : m_compare(compare), m_size(0)
    {
        reset_header();
    }

    template <typename Self>
    constexpr self_iter_t<Self> begin(this Self& self)
    {
        return iterator(as_non_const(self).header()->lchild());
    }

    template <typename Self>
    constexpr self_iter_t<Self> end(this Self& self)
    {
        return iterator(as_non_const(self).header());
    }

    constexpr size_type size() const 
    {
        return m_size;
    }

    constexpr iterator erase(const_iterator pos)
    {
        auto next = std::ranges::next(pos);
        pos.base().current->unlink(m_anchor);
        --m_size;
        return next.base();
    }

    constexpr std::pair<iterator, bool> insert(value_type& value)
    {
        auto hook = std::addressof(value.[:MemberHook:]);
        auto [x, px] = find_insert_position(hook, std::bool_constant<UniqueKey>{});

        if (px)
        {
            insert_node(x, px, hook);
            return std::make_pair(iterator(px), true);
        }
        return std::make_pair(iterator(x), false);
    }

    // comp(node_val, key) => node_val < key
    template <typename Self, typename KeyType, typename KeyTypeKeyCompare>
    constexpr self_iter_t<Self> lower_bound(this Self&& self, const KeyType& value, KeyTypeKeyCompare comp)
    {
        auto node = bs_lower_bound(as_non_const(self).header()->parent(), value, comp);
        return node == nullptr ? self.end() : iterator(node);
    }

    template <typename Self, typename KeyType, typename KeyTypeKeyCompare>
    constexpr self_iter_t<Self> upper_bound(this Self&& self, const KeyType& value, KeyTypeKeyCompare comp)
    {
        auto node = bs_upper_bound(as_non_const(self).header()->parent(), value, comp);
        return node == nullptr ? self.end() : iterator(node);
    }

    template <typename Self, typename KeyType, typename KeyTypeKeyCompare>
    constexpr std::pair<self_iter_t<Self>, self_iter_t<Self>> equal_range(this Self&& self, const KeyType& value, KeyTypeKeyCompare comp)
    {
        return { self.lower_bound(value, comp), self.upper_bound(value, comp) };
    }

    constexpr void clear()
    {
        clear_node(header()->parent());
        m_size = 0;
        reset_header();
    }

    template <typename Self>
    constexpr copy_const_to_ptr_t<Self, hook_type> root(this Self& self)
    {
        return self.header()->parent();
    }

private:

    template <typename KeyType, typename KeyTypeKeyCompare>
    constexpr iterator lower_bound_impl(const KeyType& value, KeyTypeKeyCompare comp)
    {
        auto y = header();
        auto x = header()->parent();

        while (x)
        {
            if (!comp(x, value))
            {
                y = x;
                x = x->lchild();
            }
            else
            {
                x = x->rchild();
            }
        }

        return iterator(y);
    }

    template <typename KeyType, typename KeyTypeKeyCompare>
    constexpr iterator upper_bound_impl(const KeyType& value, KeyTypeKeyCompare comp)
    {
        auto y = header();
        auto x = header()->parent();

        while (x)
        {
            if (comp(value, x))
            {
                y = x;
                x = x->lchild();
            }
            else
            {
                x = x->rchild();
            }
        }

        return iterator(y);
    }

    constexpr void reset_header()
    {
        header()->parent(nullptr);
        header()->lchild(header());
        header()->rchild(header());
        header()->as_sentinel();
    }

    constexpr void clear_node(hook_type* node)
    {
        if (!node) return;
        clear_node(node->lchild());
        clear_node(node->rchild());
        node->reset(nullptr);
    }

    template <typename Self>
    constexpr copy_const_to_ptr_t<Self, hook_type> header(this Self& self)
    {
        return std::addressof(self.m_anchor);
    }

    constexpr void insert_node(hook_type* x, hook_type* px, hook_type* node)
    {
        assert(x == nullptr && "It seems in our tree implementation, x must be nullptr.");
        bool insert_left = (x != nullptr || px == header()) || compare_hook(node, px);
        node->link(insert_left, px, m_anchor);
        ++m_size;
    }

    /**
     * @brief Finds the position to insert a node in the tree.
     * 
     * @param node The node to be inserted.
     * @param std::true_type Indicates that the insertion should be unique.
     * @return A pair of hook_type pointers indicating the insert position.
     */
    constexpr std::pair<hook_type*, hook_type*> find_insert_position(hook_type* node, std::true_type)
    {
        auto px = header();
        auto x = header()->parent();
        bool comp = true;

        while (x)
        {
            px = x;
            comp = compare_hook(node, x);
            x = comp ? x->lchild() : x->rchild();
        }

        auto y = px;

        if (comp)
        {
            if (y == header()->lchild())
            {
                return { x, px };
            }
            else
            {
                y = y->decrement();
            }
        }

        if (compare_hook(y, node))
        {
            return { x, px };
        }

        // The key conflict, the node already exists in the tree
        return { y, nullptr };
    }

    template <typename K>
    constexpr std::pair<hook_type*, bool> find_insert_position(hook_type* node, std::false_type)
    {
        auto y = header();
        auto x = header()->parent();
        bool comp = true;

        while (x)
        {
            y = x;
            comp = compare_hook(node, x); 
            x = comp ? x->lchild() : x->rchild();
        }

        return { y, comp };
    }

    constexpr bool compare_hook(hook_type* lhs, hook_type* rhs) 
    {
        auto lp = from_hook<MemberHook>(lhs);
        auto rp = from_hook<MemberHook>(rhs);
        return m_compare(*lp, *rp);
    }

    Compare m_compare;
    hook_type m_anchor;
    size_type m_size;
};

}  // namespace cpp::collections::intrusive