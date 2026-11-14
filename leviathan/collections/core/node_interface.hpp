#pragma once

#include <cassert>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/annotation.hpp>
#include <leviathan/extc++/concepts.hpp>

namespace cpp::collections
{

template <std::meta::info Member, typename Hook>
constexpr auto from_hook(Hook* hook)
{
    assert(hook != nullptr);
    using T = typename [:parent_of(Member):];
    return reinterpret_cast<T*>(reinterpret_cast<char*>(hook) - std::meta::offset_of(Member).bytes);
}

struct node_interface
{
    // For forward list nodes, the next pointer is stored in links[0]
    template <typename Node>
    constexpr auto& next(this Node& self)
    {
        return self.links[0];
    }

    template <typename Node>
    constexpr void next(this Node& self, std::type_identity_t<Node>* node)
    {
        self.next() = node;
    }

    // For bidirectional list nodes, the previous pointer is stored in links[1]
    template <typename Node>
    constexpr auto& prev(this Node& self)
    {
        return self.links[1];
    }

    template <typename Node>
    constexpr void prev(this Node& self, std::type_identity_t<Node>* node)
    {
        self.prev() = node;
    }

    // For tree nodes, the parent pointer is stored in links[0], the left child in links[1], and the right child in links[2]
    template <typename Node>
    constexpr auto& parent(this Node& self)
    {
        return self.links[0];
    }

    template <typename Node>
    constexpr void parent(this Node& self, std::type_identity_t<Node>* node)
    {
        self.parent() = node;
    }

    template <typename Node>
    constexpr auto& left(this Node& self)
    {
        return self.links[1];
    }

    template <typename Node>
    constexpr void left(this Node& self, std::type_identity_t<Node>* node)
    {
        self.left() = node;
    }

    template <typename Node>
    constexpr auto& right(this Node& self)
    {
        return self.links[2];
    }

    template <typename Node>
    constexpr void right(this Node& self, std::type_identity_t<Node>* node)
    {
        self.right() = node;
    }

    template <typename Node>
    constexpr void reset(this Node& self)
    {
        std::ranges::fill(self.links, std::addressof(self));
    }

    template <typename Node>
    constexpr void reset(this Node& self, std::type_identity_t<Node>* value)
    {
        std::ranges::fill(self.links, value);
    }
};

/**
 * @brief Binary list node interface for intrusive linked lists.
 * @note This interface assumes that the derived node structure contains an 
 *  array of two pointers named `links`.
 */
struct list_node_interface : node_interface
{
    /**
     * @brief Check if the node is linked to any other node
     * @return true if the node is linked, false otherwise
     */
    template <typename Node>
    constexpr bool is_linked(this const Node& self) 
    {
        // Both links point to self if not linked
        return !(self.prev() == std::addressof(self) && self.next() == std::addressof(self));
    }

    template <typename Node>
    constexpr void unlink(this Node& self)
    {
        if (self.is_linked())
        {
            self.next()->prev(self.prev());
            self.prev()->next(self.next());
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

        // pos - self
        auto x = std::addressof(self);
        pos->next(x);
        pos->prev(x->prev());
        x->prev()->next(pos);
        x->prev(pos);
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

        // self - pos
        auto x = std::addressof(self);
        pos->prev(x);
        pos->next(x->next());
        x->next()->prev(pos);
        x->next(pos);
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
 * - parent point to the root
 * - lchild point to the leftmost
 * - rchild point to the rightmost
 * 
 * For leftmost node where:
 * - lchild point to the header
 * 
 * For rightmost node where:
 * - rchild point to the header
 * 
 * For root node where:
 * - parent point to the header
 * 
 */
struct tree_node_interface
{
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

    /**
     * @brief Insert a node into the tree and update the header links accordingly.
     * @param self The node to be inserted
     * @param insert_left Whether to insert as the left child
     * @param parent The parent node under which to insert
     * @param header The header node containing the root, leftmost, and rightmost links
     * 
     * @note The header node must have its links properly initialized before calling this function.
     *  And the parent node should be leaf or header whose lchild and rchild should not
     *  point any node of tree.
     */
    template <typename Node>
    void insert_node_and_update_header(this Node& self, bool insert_left, Node* parent, Node& header)
    {
        auto x = std::addressof(self);
        auto& [root, leftmost, rightmost] = header.links; 

        x->parent(parent);

        // First node is always inserted left.
        if (insert_left)
        {
            parent->lchild(x);

            if (parent == &header)
            {
                root = rightmost = x;
            }
            else if (parent == leftmost)
            {
                leftmost = x;
            }
        }
        else
        {
            parent->rchild(x);

            if (parent == rightmost)
            {
                rightmost = x;
            }
        }
    }

};

/**
 * @brief Iterator for list nodes
 *  The T type must contain a member of type Hook that links the nodes together.
 * 
 * @tparam T The type of the container element
 * @tparam Hook The hook type used for linking nodes
 */
template <std::meta::info Hook>
struct list_node_iterator
{
    using hook_type = typename [:type_of(Hook):];
    using value_type = typename [:parent_of(Hook):];
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    pointer current;

    constexpr list_node_iterator() : current(nullptr) { }
    constexpr list_node_iterator(pointer node) : current(node) { }
    constexpr list_node_iterator(const list_node_iterator&) = default;

    constexpr reference operator*() const { return *current; }
    constexpr pointer operator->() const { return current; }

    constexpr list_node_iterator& operator++()
    {
        current = from_hook<Hook>(current->[:Hook:].next());
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
        current = from_hook<Hook>(current->[:Hook:].prev());
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


template <std::meta::info Hook>
struct tree_node_iterator
{
    using hook_type = typename [:type_of(Hook):];
    using value_type = typename [:parent_of(Hook):];
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    pointer current;

    constexpr tree_node_iterator lchild() const
    {
        return tree_node_iterator(from_hook<Hook>(current->[:Hook:].lchild()));
    }
    
    constexpr tree_node_iterator rchild() const
    {
        return tree_node_iterator(from_hook<Hook>(current->[:Hook:].rchild()));
    }

    constexpr tree_node_iterator parent() const
    {
        return tree_node_iterator(from_hook<Hook>(current->[:Hook:].parent()));
    }

    constexpr tree_node_iterator() : current(nullptr) { }
    constexpr tree_node_iterator(pointer node) : current(node) { }
    constexpr tree_node_iterator(const tree_node_iterator&) = default;

    constexpr reference operator*() const { return *current; }
    constexpr pointer operator->() const { return current; }

    constexpr tree_node_iterator& operator++()
    {
        current = from_hook<Hook>(current->[:Hook:].increment());
        return *this;
    }

    constexpr tree_node_iterator operator++(int)
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    constexpr tree_node_iterator& operator--()
    {
        current = from_hook<Hook>(current->[:Hook:].decrement());
        return *this;
    }

    constexpr tree_node_iterator operator--(int)
    {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    constexpr bool operator==(const tree_node_iterator&) const = default;

};


namespace detail
{

/**
 * @brief Sorts the linked list from head to tail using the provided comparison function.
 * @tparam ListNode The inner node type used in the linked list iterator.
 * @tparam Compare The type of the comparison function.
 * @param head Pointer to the first node of the list.
 * @param tail Pointer to the last node of the list.
 * @param comp Comparison function to determine the order of elements.
 */
template <typename ListNode, typename Compare>
constexpr void sort(ListNode* head, ListNode* tail, Compare comp)
{

} 

}  // namespace detail


}  // namespace cpp::collections
