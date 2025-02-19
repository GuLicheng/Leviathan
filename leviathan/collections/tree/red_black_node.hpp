#pragma once

#include "tree_node_operation.hpp"

#include <algorithm>

namespace leviathan::collections
{

struct red_black_node : binary_node_operation
{
    // Different from standard red black tree, we add another color sentinel, 
    // for each node except header, the color is red or black. And in this way, 
    // the iterator can be a circle. Since compiler will fill some bytes for 
    // struct, it will always occupy sizeof(red_black_node*) bytes. 
    enum class color { red, black, sentinel };

    red_black_node* m_link[3];

    color m_color;

    std::string to_string() const
    {
        switch (m_color)
        {
            case color::red: return "R";
            case color::black: return "B";
            default: return "S";
        }
    }

    void init()
    {
        m_color = color::red;
    }

    void as_empty_tree_header()
    {
        m_color = color::sentinel;
    }

    void clone(const red_black_node* x)
    {
        m_color = x->m_color;
    }

    bool is_header() const
    {
        return m_color == color::sentinel;
    }

    // Write insert_and_rebalance with red black tree fixup
    void insert_and_rebalance(bool insert_left, red_black_node* p, red_black_node& header)
    {
        this->insert_node_and_update_header(insert_left, p, header);

        auto& root = header.parent();
        auto x = this;

        // Rebalance
        while (x != root && x->parent()->m_color == color::red)
        {
            auto grand = x->parent()->parent();

            // The x's parent is left child
            if (x->parent() == grand->lchild())
            {
                auto uncle = grand->rchild();

                if (uncle && uncle->m_color == color::red)
                {
                    x->parent()->m_color = color::black;
                    uncle->m_color = color::black;
                    x->parent()->parent()->m_color = color::red;
                    x = x->parent()->parent();
                }
                else
                {
                    if (x == x->parent()->rchild())
                    {
                        x = x->parent();
                        x->rotate_left(root);
                    }

                    x->parent()->m_color = color::black;
                    x->parent()->parent()->m_color = color::red;
                    x->parent()->parent()->rotate_right(root);
                }
            }
            else
            {
                auto uncle = grand->lchild();

                if (uncle && uncle->m_color == color::red)
                {
                    x->parent()->m_color = color::black;
                    uncle->m_color = color::black;
                    x->parent()->parent()->m_color = color::red;
                    x = x->parent()->parent();
                }
                else
                {
                    if (x == x->parent()->lchild())
                    {
                        x = x->parent();
                        x->rotate_right(root);
                    }

                    x->parent()->m_color = color::black;
                    x->parent()->parent()->m_color = color::red;
                    x->parent()->parent()->rotate_left(root);
                }
            }
        }

        // maintain the root color
        root->m_color = color::black;
        header.m_color = color::sentinel;
    }

    // Write rebalance_for_erase with red black tree fixup
    red_black_node* rebalance_for_erase(red_black_node& header)
    {
        auto x = std::addressof(self);
        auto& [root, leftmost, rightmost] = header.m_link;
        red_black_node* successor = x;
        red_black_node* child = nullptr;
        red_black_node* child_parent = nullptr;

        if (!successor->lchild())
        {
            // If current node has no left child, it has at most one non-null child.
            // The child may be nullptr.
            child = successor->rchild();
        }
        else if (!successor->rchild())
        {
            // If current node has no right child, it has exactly one non-null child.
            // The child cannot be nullptr. 
            child = successor->lchild();
        }
        else
        {
            // If current node has both children, find successor.
            // The successor cannot be nullptr. 
            successor = successor->rchild()->minimum();

            // The child may be nullptr.
            child = successor->rchild();
        }

        if (successor != x)
        {
            // Relink successor in place of successor.
            x->lchild()->parent(successor);
            successor->lchild(x->lchild());

            if (successor != x->rchild())
            {
                child_parent = successor->parent();

                if (child)
                {
                    child->parent(successor->parent());
                }

                successor->parent()->lchild(child);
                successor->rchild(x->rchild());
                x->rchild()->parent(successor);
            }
            else
            {
                // The x only has left child.
                child_parent = successor;
            }

            if (x == root)
            {
                root = successor;
            }
            else if (x == x->parent()->lchild())
            {
                x->parent()->lchild(successor);
            }
            else
            {
                x->parent()->rchild(successor);
            }

            successor->parent(x->parent());
            std::swap(successor->m_color, x->m_color);
            successor = x;
        }
        else
        {
            // The x is left node or just has one child.
            child_parent = successor->parent();

            if (child)
            {
                child->parent(successor->parent());
            }

            if (root == x)
            {
                root = child;
            }
            else if (x == x->parent()->lchild())
            {
                x->parent()->lchild(child);
            }
            else
            {
                x->parent()->rchild(child);
            }

            // Maintain the leftmost
            if (x == leftmost)
            {
                // The x is leaf node or only has right child.
                leftmost = x->rchild() ? x->parent() : successor->minimum();
            }

            // Maintain the rightmost
            if (x == rightmost)
            {
                // The x is leaf node or only has left child.
                rightmost = x->lchild() ? x->parent() : successor->maximum();
            }
        }


        // Rebalance
    }

};

}  // namespace leviathan::collections 



