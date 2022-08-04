
#pragma once

#include <assert.h>
#include <concepts>

namespace leviathan::collections 
{

    // Some basic operations on binary search tree node

    template <typename TreeNode>
    struct tree_node_basic_operation
    {
        constexpr static TreeNode* increment(TreeNode* x)
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

        constexpr static TreeNode* decrement(TreeNode* x)
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

        constexpr static const TreeNode* increment(const TreeNode* x)
        { return increment(const_cast<TreeNode*>(x)); }

        constexpr static const TreeNode* decrement(const TreeNode* x)
        { return decrement(const_cast<TreeNode*>(x)); }

        constexpr static TreeNode* maximum(TreeNode* x)
        {
            assert(x && "x should not be nullptr");
            for (; x->m_right; x = x->m_right);
            return x;
        }

        constexpr static TreeNode* minimum(TreeNode* x)
        {
            assert(x && "x should not be nullptr");
            for (; x->m_left; x = x->m_left);
            return x;
        }

        constexpr static const TreeNode* maximum(const TreeNode* x)
        { return maximum(const_cast<TreeNode*>(x)); }

        constexpr static const TreeNode* minimum(const TreeNode* x)
        { return minimum(const_cast<TreeNode*>(x)); }

        /*
        *     x            y              
        *       \   =>   /    
        *         y    x
        */
        constexpr static void tree_rotate_left(TreeNode* x, TreeNode*& root)
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
        constexpr static void tree_rotate_right(TreeNode* x, TreeNode*& root)
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

        constexpr static TreeNode* left(TreeNode* x) 
        { return x->m_left; }

        constexpr static TreeNode* right(TreeNode* x) 
        { return x->m_right; }

        constexpr static TreeNode* parent(TreeNode* x) 
        { return x->m_parent; }

        constexpr static const TreeNode* left(const TreeNode* x) 
        { return x->m_left; }

        constexpr static const TreeNode* right(const TreeNode* x) 
        { return x->m_right; }

        constexpr static const TreeNode* parent(const TreeNode* x) 
        { return x->m_parent; }

        constexpr static void set_left(TreeNode* x, TreeNode* left)
        { x->m_left = left; }

        constexpr static void set_right(TreeNode* x, TreeNode* right)
        { x->m_right = right; }

        constexpr static void set_parent(TreeNode* x, TreeNode* parent)
        { x->m_parent = parent; }

        TreeNode* m_parent;
        TreeNode* m_left;
        TreeNode* m_right;
    };

    /*
        class Node
        {
            void insert_and_rebalance(bool insert_left, avl_node_base* x, avl_node_base* p, avl_node_base& header);
            Node* rebalance_for_erase(avl_node_base* z, avl_node_base& header);
        };
    */

}