#pragma once

#include "tree_node_operation.hpp"

#include <algorithm>

namespace leviathan::collections
{

struct red_black_node : binary_node_operation
{
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

        // Rebalance
        while (x != root && x->parent()->m_color == color::red)
        {
            // The x's parent is left child
            if (x->parent() == x->parent()->parent()->lchild())
            {
                auto y = x->parent()->parent()->rchild();

                if (y && y->m_color == color::red)
                {
                    x->parent()->m_color = color::black;
                    y->m_color = color::black;
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
                auto y = x->parent()->parent()->lchild();

                if (y->m_color == color::red)
                {
                    x->parent()->m_color = color::black;
                    y->m_color = color::black;
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

        root->m_color = color::black;
        header.m_color = color::sentinel;
    }

    // Write rebalance_for_erase with red black tree fixup
    red_black_node* rebalance_for_erase(red_black_node& header);

};

}  // namespace leviathan::collections 



