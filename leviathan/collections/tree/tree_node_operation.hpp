/*
    Since each node will occupy memory allocated by allocator,
    the node will always be lvalue.
*/
#pragma once

#include <type_traits>
#include <memory>
#include <format>
#include <assert.h>

namespace leviathan::collections
{

// The operation is based on tree.hpp
struct basic_tree_node_operation
{
    template <typename Node>
    auto& parent(this Node& self)
    {
        return self.m_link[0];
    }

    template <typename Node>
    auto& lchild(this Node& self)
    {
        return self.m_link[1];
    }

    template <typename Node>
    auto& rchild(this Node& self)
    {
        return self.m_link[2];
    }

    // The std::type_identity_t is used to avoid ambiguity for nullptr.
    template <typename Node>
    void parent(this Node& self, std::type_identity_t<Node>* node)
    {
        self.m_link[0] = node;
    }

    template <typename Node>
    void lchild(this Node& self, std::type_identity_t<Node>* node)
    {
        self.m_link[1] = node;
    }

    template <typename Node>
    void rchild(this Node& self, std::type_identity_t<Node>* node)
    {
        self.m_link[2] = node;
    }
};

// The operation is based on tree.hpp
struct binary_node_operation : basic_tree_node_operation
{
    // Return rightmost of binary node.
    template <typename Node>
    constexpr Node* maximum(this Node& self)
    {
        auto x = std::addressof(self);
        for (; x->rchild(); x = x->rchild());
        return x;
    }

    // Return leftmost of binary node.
    template <typename Node>
    constexpr Node* minimum(this Node& self)
    {
        auto x = std::addressof(self);
        for (; x->lchild(); x = x->lchild());
        return x;
    }

    // Return prev position of binary node.
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

    // Return next position of binary node.
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

    /*
    *     x            y              
    *       \   =>   /    
    *         y    x
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

    /*
    *     x        y                   
    *    /     =>    \
    *   y              x
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

    // Insert node to binary tree and update header.
    template <typename Node>
    void insert_node_and_update_header(this Node& self, bool insert_left, Node* parent, Node& header)
    {
        auto x = std::addressof(self);
        auto& [root, leftmost, rightmost] = header.m_link; 

        x->parent(parent);

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

    // Remove node
    template <typename Node>
    void replace_node_with_successor(this Node& self, Node& header);
};

}
