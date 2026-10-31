#pragma once

#include <cassert>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <meta>

namespace cpp::collections
{

/**
 * @brief Binary list node interface for intrusive linked lists.
 * @note This interface assumes that the derived node structure contains an 
 *  array of two pointers named `links`.
 */
struct list_node_interface
{
    template <typename Node>
    constexpr auto& prev(this Node& self)
    {
        return self.links[0];
    }

    // The std::type_identity_t is used to avoid ambiguity for nullptr.
    template <typename Node>
    constexpr void prev(this Node& self, std::type_identity_t<Node>* node)
    {
        self.links[0] = node;
    }

    template <typename Node>
    constexpr auto& next(this Node& self)
    {
        return self.links[1];
    }

    template <typename Node>
    constexpr void next(this Node& self, std::type_identity_t<Node>* node)
    {
        self.links[1] = node;
    }

    /**
     * @brief Reset the node's links to point to itself, indicating it is not linked.
     */
    template <typename Node>
    constexpr void reset(this Node& self)
    {
        self.links[0] = self.links[1] = std::addressof(self);
    }

    /**
     * @brief Check if the node is linked to any other node
     * @return true if the node is linked, false otherwise
     */
    template <typename Node>
    constexpr bool is_linked(this const Node& self) 
    {
        // Both links point to self if not linked
        return !(self.links[0] == std::addressof(self) && self.links[1] == std::addressof(self));
    }

    template <typename Node>
    constexpr void unlinked(this Node& self)
    {
        if (self.is_linked())
        {
            self.prev(self.prev()->prev());
            self.next(self.next()->next());
            self.reset();
        }
    }

    /**
     * @brief Insert self after the given position node
     * @param pos The position node after which self will be inserted
     * 
     * @note The node must not be linked to any other list.
     */
    template <typename Node>
    constexpr void insert_after(this Node& self, std::type_identity_t<Node>* pos)
    {
        assert(pos != nullptr && pos != std::addressof(self));

        self.prev(pos);
        self.next(pos->next());
        pos->next()->prev(std::addressof(self));
        pos->next(std::addressof(self));
    }

    /**
     * @brief Insert self before the given position node
     * @param pos The position node before which self will be inserted
     * 
     * @note The node must not be linked to any other list.
     */
    template <typename Node>
    constexpr void insert_before(this Node& self, std::type_identity_t<Node>* pos)
    {
        assert(pos != nullptr && pos != std::addressof(self));

        self.next(pos);
        self.prev(pos->prev());
        pos->prev()->next(std::addressof(self));
        pos->prev(std::addressof(self));
    }

};

/**
 * @brief Interface for tree nodes, providing access to parent, left child, and right child links.
 * @note This interface assumes that the derived node structure contains an 
 *  array of three pointers named `links`.
 * 
 * Our node contains an array of links where:
 * - links[0] represents the parent node
 * - links[1] represents the left child node
 * - links[2] represents the right child node
 * 
 * The tree must contains header where:
 * - links[0] point to the root
 * - links[1] point to the leftmost
 * - links[2] point to the rightmost
 * 
 * For leftmost node where:
 * - links[1] point to the header
 * 
 * For rightmost node where:
 * - links[2] point to the header
 * 
 * For root node where:
 * - links[0] point to the header
 * 
 */
struct tree_node_interface
{
    template <typename Node>
    constexpr auto& parent(this Node& self)
    {
        return self.links[0];
    }

    template <typename Node>
    constexpr auto& lchild(this Node& self)
    {
        return self.links[1];
    }

    template <typename Node>
    constexpr auto& rchild(this Node& self)
    {
        return self.links[2];
    }

    template <typename Node>
    constexpr void parent(this Node& self, std::type_identity_t<Node>* node)
    {
        self.links[0] = node;
    }

    template <typename Node>
    constexpr void lchild(this Node& self, std::type_identity_t<Node>* node)
    {
        self.links[1] = node;
    }

    template <typename Node>
    constexpr void rchild(this Node& self, std::type_identity_t<Node>* node)
    {
        self.links[2] = node;
    }

    template <typename Node>
    constexpr void reset(this Node& self)
    {
        self.links[0] = std::addressof(self);
        self.links[1] = std::addressof(self);
        self.links[2] = std::addressof(self);
    }

    /**
     * @brief Find the maximum (rightmost) node in the subtree rooted at self
     * @return Pointer to the maximum node
     */
    template <typename Node>
    constexpr Node* maximum(this Node& self)
    {
        auto x = std::addressof(self);
        for (; x->rchild(); x = x->rchild());
        return x;
    }

    /**
     * @brief Find the minimum (leftmost) node in the subtree rooted at self
     * @return Pointer to the minimum node
     */
    template <typename Node>
    constexpr Node* minimum(this Node& self)
    {
        auto x = std::addressof(self);
        for (; x->lchild(); x = x->lchild());
        return x;
    }

    /**
     * @brief Find the predecessor of the binary node (the previous node in in-order traversal)
     * @return Pointer to the predecessor node
     */
    template <typename Node>
    constexpr Node* decrement(this Node& self)
    {
        auto x = std::addressof(self);

        if (x->lchild())
        {
            return x->lchild()->maximum();
        }
        else
        {
            auto y = x->parent();

            // y will never be nullptr 
            while (x == y->lchild())
            {
                x = y;
                y = y->parent();
            }

            if (x->lchild() != y)
            {
                x = y;
            }

            return x;
        }
    }

    /**
     * @brief Find the successor of the binary node (the next node in in-order traversal)
     * @return Pointer to the successor node
     */
    template <typename Node>
    constexpr Node* increment(this Node& self)
    {
        auto x = std::addressof(self);

        if (x->rchild())
        {
            return x->rchild()->minimum();
        }
        else
        {
            auto y = x->parent();

            // y will never be nullptr
            while (x == y->rchild())
            {
                x = y;
                y = y->parent();
            }

            if (x->rchild() != y)
            {
                x = y;
            }

            return x;
        }
    }

        /**
     * @brief Perform a left rotation on the node
     * @param root Reference to the root of the tree
     * @note The tree must maintain proper parent and child links for this operation to work correctly.
     * 
     * @note After rotation, the node `self` will become the left child of its right child.
     *       x            y              
     *         \   =>   /    
     *          y    x
     */
    template <typename Node>    
    void rotate_left(this Node& self, Node*& root)
    {
        auto x = std::addressof(self);
        auto y = x->rchild();

        x->rchild(y->lchild());

        if (y->lchild())
        {
            y->lchild()->parent(x);
        }

        y->parent(x->parent());

        if (x == root)
        {
            root = y;
        }
        else if (x == x->parent()->lchild()) 
        {
            x->parent()->lchild(y);
        }
        else
        {
            x->parent()->rchild(y);
        }
        
        y->lchild(x);
        x->parent(y);
    }

    /**
     * @brief Perform a right rotation on the node
     * @param root Reference to the root of the tree
     * @note The tree must maintain proper parent and child links for this operation to work correctly.
     * 
     * @note After rotation, the node `self` will become the right child of its left child.
     *       x          y                   
     *      /     =>     \
     *     y              x
     */
    template <typename Node>
    void rotate_right(this Node& self, Node*& root)
    {
        auto x = std::addressof(self);
        auto y = x->lchild();

        x->lchild(y->rchild());
        
        if (y->rchild())
        {
            y->rchild()->parent(x);
        }

        y->parent(x->parent());

        if (x == root)
        {
            root = y;
        }
        else if (x == x->parent()->rchild())
        {
            x->parent()->rchild(y);
        }
        else
        {
            x->parent()->lchild(y);
        }
        
        y->rchild(x);
        x->parent(y);
    }

};

consteval std::meta::info search_hook(std::meta::info type, std::meta::info hook)
{
    constexpr auto ctx = std::meta::access_context::current(); 
    auto members = std::meta::nonstatic_data_members_of(type, ctx);
    return *std::ranges::find_if(
        members,
        std::bind_front(std::meta::is_base_of_type, std::meta::dealias(hook)),
        std::meta::type_of
    );
}

template <typename Hook, typename T>
constexpr auto& to_hook(T& obj)
{
    constexpr auto type = std::meta::remove_cvref(^^T);
    constexpr auto info = search_hook(type, ^^Hook);
    return obj.[:info:];
}

template <typename T, typename Hook>
constexpr T* from_hook(Hook* hook)
{
    assert(hook != nullptr);
    return reinterpret_cast<T*>(reinterpret_cast<char*>(hook) - std::meta::offset_of(^^T::Hook).bytes);
}

// Member
template <typename T>
struct list_node_iterator
{
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    static constexpr auto hook = search_hook(^^T, ^^list_node_interface);

    pointer current;

    constexpr list_node_iterator() : current(nullptr) { }
    constexpr list_node_iterator(pointer node) : current(node) { }
    constexpr list_node_iterator(const list_node_iterator&) = default;

    constexpr reference operator*() const { return *current; }
    constexpr pointer operator->() const { return current; }

    constexpr list_node_iterator& operator++()
    {
        current = from_hook<T>(current->[:hook:].next());
        return *this;
    }

    constexpr list_node_iterator operator++(int)
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    constexpr list_node_iterator& operator--()
    {
        current = from_hook<T>(current->[:hook:].prev());
        return *this;
    }

    constexpr list_node_iterator operator--(int)
    {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    constexpr bool operator==(const list_node_iterator&) const = default;
};

}  // namespace cpp::collections
