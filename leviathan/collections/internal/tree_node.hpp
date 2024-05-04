
#pragma once

#include <assert.h>
#include <concepts>

namespace leviathan::collections 
{

// Some basic operations on binary search tree node

template <typename TreeNode>
struct tree_node_basic_operation
{
    static constexpr TreeNode* increment(TreeNode* x)
    {
        assert(x && "x should not be nullptr");
        if (x->m_right)
        {
            x = minimum(x->m_right);
        }
        else
        {
            TreeNode* y = x->m_parent;
            while (x == y->m_right)
            {
                x = y;
                y = y->m_parent;
            }
            if (x->m_right != y)
                x = y;
        }
        return x;
    }

    static constexpr TreeNode* decrement(TreeNode* x)
    {
        assert(x && "x should not be nullptr");
        if (x->m_left)
        {
            x = maximum(x->m_left);
        }
        else
        {
            TreeNode* y = x->m_parent;
            while (x == y->m_left)
            {
                x = y;
                y = y->m_parent;
            }
            if (x->m_left != y)
                x = y;
        }
        return x;
    }

    static constexpr const TreeNode* increment(const TreeNode* x)
    { return increment(const_cast<TreeNode*>(x)); }

    static constexpr const TreeNode* decrement(const TreeNode* x)
    { return decrement(const_cast<TreeNode*>(x)); }

    static constexpr TreeNode* maximum(TreeNode* x)
    {
        assert(x && "x should not be nullptr");
        for (; x->m_right; x = x->m_right);
        return x;
    }

    static constexpr TreeNode* minimum(TreeNode* x)
    {
        assert(x && "x should not be nullptr");
        for (; x->m_left; x = x->m_left);
        return x;
    }

    static constexpr const TreeNode* maximum(const TreeNode* x)
    { return maximum(const_cast<TreeNode*>(x)); }

    static constexpr const TreeNode* minimum(const TreeNode* x)
    { return minimum(const_cast<TreeNode*>(x)); }

    /*
    *     x            y              
    *       \   =>   /    
    *         y    x
    */
    static constexpr void tree_rotate_left(TreeNode* x, TreeNode*& root)
    {
        TreeNode* y = x->m_right;

        x->m_right = y->m_left;
        if (y->m_left != 0)
            y->m_left->m_parent = x;
        y->m_parent = x->m_parent;

        // x->parent will never be nullptr, since header->parent == root and root->parent == header
        if (x == root)
            root = y;
        else if (x == x->m_parent->m_left) 
            x->m_parent->m_left = y;
        else
            x->m_parent->m_right = y;
        y->m_left = x;
        x->m_parent = y;
    }


    /*
    *     x        y                   
    *    /     =>    \
    *   y              x
    */
    static constexpr void tree_rotate_right(TreeNode* x, TreeNode*& root)
    {
        TreeNode* y = x->m_left;

        x->m_left = y->m_right;
        if (y->m_right != 0)
            y->m_right->m_parent = x;
        y->m_parent = x->m_parent;

        if (x == root)
            root = y;
        else if (x == x->m_parent->m_right)
            x->m_parent->m_right = y;
        else
            x->m_parent->m_left = y;
        y->m_right = x;
        x->m_parent = y;
    }

    // Observers and modifiers
    static constexpr TreeNode* left(TreeNode* x) 
    { return x->m_left; }

    static constexpr TreeNode* right(TreeNode* x) 
    { return x->m_right; }

    static constexpr TreeNode* parent(TreeNode* x) 
    { return x->m_parent; }

    static constexpr const TreeNode* left(const TreeNode* x) 
    { return x->m_left; }

    static constexpr const TreeNode* right(const TreeNode* x) 
    { return x->m_right; }

    static constexpr const TreeNode* parent(const TreeNode* x) 
    { return x->m_parent; }

    static constexpr void set_left(TreeNode* x, TreeNode* left)
    { x->m_left = left; }

    static constexpr void set_right(TreeNode* x, TreeNode* right)
    { x->m_right = right; }

    static constexpr void set_parent(TreeNode* x, TreeNode* parent)
    { x->m_parent = parent; }

    TreeNode* m_parent;
    TreeNode* m_left;
    TreeNode* m_right;
};

template <typename Node>
concept tree_node_interface = requires(Node *node1, Node* node2, const Node *cnode, bool insert_left, Node& header)
{
    // insert node1 to left of node2 if insert_left is true, otherwise right
    { Node::insert_and_rebalance(insert_left, node1, node2, header) } -> std::same_as<void>;  

    // remove node1 and rebalance tree if necessary
    { Node::rebalance_for_erase(node1, header) } -> std::same_as<Node*>;  

    // check whether node1 is a header node, see end of this file
    { Node::is_header(node1) } -> std::same_as<bool>;

    // predecessor and successor
    { Node::increment(node1) } -> std::same_as<Node*>;  // the prev node of node1
    { Node::decrement(node1) } -> std::same_as<Node*>;  // the next node of node1

    // max and min
    { Node::maximum(node1) } -> std::same_as<Node*>;  // leftmost of node1
    { Node::minimum(node1) } -> std::same_as<Node*>;  // rightmost of node1

    // setter and getter
    { Node::set_left(node1, node2) } -> std::same_as<void>;    // equivalent to node1->left = node2
    { Node::set_right(node1, node2) } -> std::same_as<void>;   // equivalent to node1->right = node2
    { Node::set_parent(node1, node2) } -> std::same_as<void>;  // equivalent to node1->parent = node2

    { Node::left(node1) } -> std::same_as<Node*>;    // equivalent to return node1->left
    { Node::right(node1) } -> std::same_as<Node*>;   // equivalent to return node1->right
    { Node::parent(node1) } -> std::same_as<Node*>;  // equivalent to return node1->parent

    // clone node from other
    // for AVL tree, it may copy balanced_factor
    // for RB tree, it may copy color
    // for SBT, it may copy size infomation 
    { Node::clone(node1, cnode) } -> std::same_as<void>;  // copy members without value field from cnode to node1

    // default constructor for node and header
    { Node::reset(node1) } -> std::same_as<void>;  // reset header for an empty tree 
    { Node::init(node1) } -> std::same_as<void>;   // init node without value field after calling allocate

    // const version
    { Node::increment(cnode) } -> std::same_as<const Node*>;
    { Node::decrement(cnode) } -> std::same_as<const Node*>;

    { Node::left(cnode) } -> std::same_as<const Node*>;
    { Node::right(cnode) } -> std::same_as<const Node*>;
    { Node::parent(cnode) } -> std::same_as<const Node*>;

    { Node::maximum(cnode) } -> std::same_as<const Node*>;
    { Node::minimum(cnode) } -> std::same_as<const Node*>;

};
}

/*
For in gcc and clang, advance an empty set::iterator will cause infinity loop:
e.g.
{
    std::set<int> s;
    s.begin()++; // infinity loop
}

To avoid this, MSVC introduce an new flag, but cost more memory for each node.

Since for an empty tree, the header->left == header->right == header, see follow code.
if (x->right)
{
    x = x->right;
    for (; x->left; x = x->left); // x->left will always be true
}

To avoid this situation, we use this method(Node::is_header) to check whether to increment the current node.
*/