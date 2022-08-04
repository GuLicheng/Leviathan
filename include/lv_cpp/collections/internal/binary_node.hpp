#pragma once

#include "tree_node.hpp"

namespace leviathan::collections
{

    struct simple_binary_node : tree_node_basic_operation<simple_binary_node>
    {
        constexpr static void reset(simple_binary_node* node)
        {
            node->m_parent = nullptr;
            node->m_left = node->m_right = node;
        }

        constexpr static void init(simple_binary_node* node)
        {
            node->m_left = node->m_right = node->m_parent = nullptr;
        }

        constexpr static void insert_and_rebalance(bool insert_left,
                                      simple_binary_node* x,
                                      simple_binary_node* p,
                                      simple_binary_node& header)
        {
            x->m_parent = p;
            x->m_left = x->m_right = nullptr;

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
        }

        constexpr static bool is_header(simple_binary_node* node)
        { return false; }


        constexpr static simple_binary_node* rebalance_for_erase(simple_binary_node* z, simple_binary_node& header)
        {
            auto x = z;

            assert(x && "x should not be nullptr");

            simple_binary_node*& root = header.m_parent;
            simple_binary_node*& leftmost = header.m_left;
            simple_binary_node*& rightmost = header.m_right;

            simple_binary_node* child = nullptr;
            simple_binary_node* parent = nullptr; // for rebalance

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
        
            return z;
        }

    };


}
