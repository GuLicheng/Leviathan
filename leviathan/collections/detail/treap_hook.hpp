#pragma once

#include <leviathan/collections/core/node_interface.hpp>

namespace cpp::collections::intrusive
{

struct treap_hook : tree_node_interface
{
    treap_hook* links[3];

    size_t priority;

    constexpr void as_sentinel()
    {
        priority = upper;
    }

    constexpr bool is_sentinel() const
    {
        return priority == upper;
    }

    constexpr void link(bool insert_left, treap_hook* p, treap_hook& header)
    {
        assert(this->priority != upper && "The node should not be header");
        this->insert_node_and_update_header(insert_left, p, header);
        priority = random_priority();

        // Rebalance
        auto x = this;
        
        while (x->priority > x->parent()->priority)
        {
            auto y = x->parent();

            x == y->lchild() ?
                y->rotate_right(header.parent()) :
                y->rotate_left(header.parent());
        }
    }

    constexpr void unlink(treap_hook& header)
    {
        auto x = this;
        auto& [root, leftmost, rightmost] = header.links;

        while (x->lchild() || x->rchild())
        {
            if (!x->rchild() || (x->lchild() && x->lchild()->priority > x->rchild()->priority))
            {
                x->rotate_right(header.parent());
            }
            else
            {
                x->rotate_left(header.parent());
            }
        }
        
        if (x == root)
        {
            root = nullptr;
            leftmost = rightmost = &header;
        }
        else
        {
            if (x == x->parent()->lchild())
            {
                x->parent()->lchild(nullptr);
                
                if (x == leftmost)
                {
                    leftmost = x->parent();
                }
            }
            else
            {
                x->parent()->rchild(nullptr);

                if (x == rightmost)
                {
                    rightmost = x->parent();
                }
            }
        }
    }

private:

    static constexpr size_t lower = 0;
    
    static constexpr size_t upper = std::numeric_limits<size_t>::max();

    inline static std::mt19937 engine{ std::random_device{}() };
    inline static std::uniform_int_distribution<size_t> distribution{ lower, upper - 1 };

    constexpr static size_t random_priority()
    {
        return distribution(engine);
    }

};

}  // namespace cpp::collections::intrusive