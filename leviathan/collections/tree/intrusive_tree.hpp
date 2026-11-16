#pragma once

#include <leviathan/collections/core/node_interface.hpp>
#include <leviathan/collections/core/container_interface.hpp>
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
    
    constexpr reference operator*() const { return *current; }
    constexpr pointer operator->() const { return current; }

    constexpr tree_hook_iterator& operator++()
    {
        current = from_hook<MemberHook>(current->[:MemberHook:].increment());
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
        current = from_hook<MemberHook>(current->[:MemberHook:].decrement());
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

template <std::meta::info MemberHook, typename TreeMemberHook, typename Compare = std::less<>>
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

    constexpr tree() : m_compare(Compare{})
    {
        m_size = 0;
        m_sentinel.color = red_black_color::sentinel;
    }

    constexpr tree(const Compare& compare) : m_compare(compare), m_size(0)
    {
        header()->parent(nullptr);
        header()->lchild(header());
        header()->rchild(header());
        header()->as_sentinel();
    }

    template <typename Self>
    constexpr self_iter_t<Self> begin(Self& self)
    {
        return iterator(as_non_const(self).header()->lchild());
    }

    template <typename Self>
    constexpr self_iter_t<Self> end(Self& self)
    {
        return iterator(as_non_const(self).header());
    }

    constexpr size_type size() const 
    {
        return m_size;
    }

    constexpr std::pair<iterator, bool> insert(value_type& value)
    {
        auto hook = std::adressof(value.[:MemberHook:]);
        auto [x, px] = find_insert_position(hook, std::true_type{});

        if (px)
        {
            insert_node(x, px, hook);
            return std::make_pair(iterator(px), true);
        }
        return std::make_pair(iterator(x), false);
    }

private:

    constexpr void insert_node(hook_type* x, hook_type* px, hook_type* node)
    {
        assert(x == nullptr && "It seems in our tree implementation, x must be nullptr.");
        bool insert_left = (x != nullptr || px == header()) || compare_hook(node, px);
        node->link(insert_left, px, m_sentinel);
        ++m_size;
    }

    /**
     * @brief Finds the position to insert a node in the tree.
     * 
     * @param node The node to be inserted.
     * @param std::true_type Indicates that the insertion should be unique.
     * @return A pair of hook_type pointers indicating the insert position.
     */
    constexpr std::pair<hook_type*, hook_type*> find_insert_position(const hook_type* node, std::true_type)
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
    constexpr std::pair<hook_type*, bool> get_insert_pos(const hook_type* node, std::false_type)
    {
        auto y = &m_header, x = header()->parent();
        bool comp = true;

        while (x)
        {
            y = x;
            comp = compare_hook(node, x); 
            x = comp ? x->lchild() : x->rchild();
        }

        return { y, comp };
    }

    constexpr bool compare_hook(const hook_type* lhs, const hook_type* rhs) const
    {
        return m_compare(from_hook<MemberHook>(lhs), from_hook<MemberHook>(rhs));
    }

    template <typename Self>
    constexpr copy_const_to_ptr_t<Self, hook_type> header(this Self& self)
    {
        return std::addressof(self.m_sentinel);
    }

    Compare m_compare;
    hook_type m_sentinel;
    size_type m_size;
};

}  // namespace cpp::collections::intrusive