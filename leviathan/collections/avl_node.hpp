#pragma once

#include "tree_node.hpp"

namespace leviathan::collections
{

struct avl_node : tree_node_basic_operation<avl_node>
{
    static constexpr void reset(avl_node* node)
    {
        node->m_parent = nullptr;
        node->m_left = node->m_right = node;
        node->m_height = -1;    // If m_height == -1, the node must be header.
    }

    static constexpr void init(avl_node* node)
    {
        node->m_left = node->m_right = node->m_parent = nullptr;
        node->m_height = 1; // the height of left is 1
    }

    static constexpr int height(const avl_node* x)
    { return x ? x->m_height : 0; }

    static constexpr void update_height(avl_node* x)
    {
        assert(x && "x should not be nullptr");
        int lh = height(x->m_left);
        int rh = height(x->m_right);
        x->m_height = std::max(lh, rh) + 1;
    }

    // remove x from tree and return removed node
    static constexpr void erase_node(avl_node* x, avl_node* header)
    {
        assert(x && "x should not be nullptr");

        avl_node*& root = header->m_parent;
        avl_node*& leftmost = header->m_left;
        avl_node*& rightmost = header->m_right;

        avl_node* child = nullptr;
        avl_node* parent = nullptr; // for rebalance

        if (x->m_left && x->m_right)
        {
            auto successor = minimum(x->m_right);
            child = successor->m_right;
            parent = successor->m_parent;
            if (child)
            {
                child->m_parent = parent;
            }

                (successor->m_parent->m_left == successor ? 
                successor->m_parent->m_left : 
                successor->m_parent->m_right) = child;


            if (successor->m_parent == x)
                parent = successor;
            
            successor->m_left = x->m_left;
            successor->m_right = x->m_right;
            successor->m_parent = x->m_parent;
            successor->m_height = x->m_height;
        
            if (x == root)
                root = successor;
            else
                (x->m_parent->m_left == x ? x->m_parent->m_left : x->m_parent->m_right) = successor;
            
            x->m_left->m_parent = successor;

            if (x->m_right)
                x->m_right->m_parent = successor;

        }
        else
        {
            // update leftmost or rightmost
            if (!x->m_left && !x->m_right) 
            {
                // leaf, such as just one root
                if (x == leftmost)
                    leftmost = x->m_parent;
                if (x == rightmost)
                    rightmost = x->m_parent;
            }
            else if (x->m_left)
            {
                // only left child
                child = x->m_left;
                if (x == rightmost)
                    rightmost = maximum(child);
            }                
            else
            {
                // only right child
                child = x->m_right;
                if (x == leftmost)
                    leftmost = minimum(child);
            }

            if (child)
                child->m_parent = x->m_parent;
            if (x == root)
                root = child;
            else
                (x->m_parent->m_left == x ? x->m_parent->m_left : x->m_parent->m_right) = child;
            parent = x->m_parent;
        }
        avl_tree_rebalance_erase(parent, header);
    }

    static constexpr void avl_tree_fix_l(avl_node* x, avl_node* header)
    {
        auto r = x->m_right;
        int lh0 = height(r->m_left);
        int rh0 = height(r->m_right);
        if (lh0 > rh0)
        {
            tree_rotate_right(r, header->m_parent);
            update_height(r);
            update_height(r->m_parent);
        }
        tree_rotate_left(x, header->m_parent);
        update_height(x);
        update_height(x->m_parent);
    }

    static constexpr void avl_tree_fix_r(avl_node* x, avl_node* header)
    {
        auto l = x->m_left;
        int lh0 = height(l->m_left);
        int rh0 = height(l->m_right);
        if (lh0 < rh0)
        {
            tree_rotate_left(l, header->m_parent);
            update_height(l);
            update_height(l->m_parent);
        }
        tree_rotate_right(x, header->m_parent);
        update_height(x);
        update_height(x->m_parent);
    }

    static constexpr void avl_tree_rebalance_insert(avl_node* x, avl_node* header)
    {
        for (x = x->m_parent; x != header; x = x->m_parent)
        {
            int lh = height(x->m_left);
            int rh = height(x->m_right);
            int h = std::max(lh, rh) + 1;
            if (x->m_height == h) break;
            x->m_height = h;

            int diff = lh - rh;
            if (diff <= -2)
            {
                avl_tree_fix_l(x, header);
            }
            else if (diff >= 2)
            {
                avl_tree_fix_r(x, header);
            }
        }
    }

    static constexpr void avl_tree_rebalance_erase(avl_node* x, avl_node* header)
    {
        for (; x != header; x = x->m_parent)
        {
            int lh = height(x->m_left);
            int rh = height(x->m_right);
            int h = std::max(lh, rh) + 1;
            x->m_height = h;

            int diff = lh - rh;
            
            if (x->m_height != h)
                x->m_height = h;
            else if (-1 <= diff && diff <= 1) 
                break;

            if (diff <= -2)
                avl_tree_fix_l(x, header);
            else 
                avl_tree_fix_r(x, header);
        }
    }

    static constexpr void insert_and_rebalance(bool insert_left,
                                    avl_node* x,
                                    avl_node* p,
                                    avl_node& header)
    {
        x->m_parent = p;
        x->m_left = x->m_right = nullptr;
        x->m_height = 1;

        if (insert_left)
        {
            p->m_left = x;
            if (p == &header)
            {
                header.m_parent = x;
                header.m_right = x;
            }
            else if (p == header.m_left)
                header.m_left = x;
        }
        else
        {
            p->m_right = x;
            if (p == header.m_right)
                header.m_right = x;
        }

        // rebalance
        avl_node::avl_tree_rebalance_insert(x, &header);
    }

    static constexpr avl_node* rebalance_for_erase(avl_node* z, avl_node& header)
    {
        erase_node(z, std::addressof(header));
        return z;
    }

    static constexpr void clone(avl_node* x, const avl_node* y)
    { x->m_height = y->m_height; } 


    static constexpr bool is_header(avl_node* node)
    { return node->m_height == -1; }

    int m_height; 
};


static_assert(tree_node_interface<avl_node>);

}
