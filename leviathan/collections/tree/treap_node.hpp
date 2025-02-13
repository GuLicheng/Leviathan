// https://medium.com/carpanese/a-visual-introduction-to-treap-data-structure-part-1-6196d6cc12ee
#pragma once

#include "tree_node_operation.hpp"

#include <random>

namespace leviathan::collections
{

template <typename RandomEngine = std::mt19937, typename SeedGenerator = std::random_device>
struct treap_node : basic_tree_node_operation, binary_node_operation
{
    static_assert(std::is_unsigned_v<typename RandomEngine::result_type>);

    // All nodes share one random generator
    inline static RandomEngine random = RandomEngine(SeedGenerator()());
    // inline static RandomEngine random = RandomEngine(2);

    // Nodes
    treap_node* m_link[3];

    // Priority of current node, -1 for sentinel
    // and the heap is a max-heap
    typename RandomEngine::result_type m_priority;

    std::string to_string() const
    {
        return std::format("{}", m_priority);
    }

    void as_empty_tree_header()
    {
        m_priority = RandomEngine::max();
    }

    static auto get_random_number()
    {
        return random() % RandomEngine::max();

        // auto number = random();
        // std::cout << "number = " << number % 100 << std::endl;
        // return number % 100;
    }

    void init()
    {
        m_priority = get_random_number();
    }

    void clone(const treap_node* node)
    {
        m_priority = node->m_priority;
    }

    bool is_header() const
    {
        return m_priority == RandomEngine::max();
    }

    void insert_and_rebalance(bool insert_left, treap_node* p, treap_node& header)
    {
        auto x = this;

        x->parent(p);
        x->lchild(nullptr);
        x->rchild(nullptr);

        if (insert_left)
        {
            p->lchild(x);
            if (p == &header)
            {
                header.parent(x);
                header.rchild(x);
            }
            else if (p == header.lchild())
            {
                header.lchild(x);
            }
        }
        else
        {
            p->rchild(x);
            if (p == header.rchild())
            {
                header.rchild(x);
            }
        }

        // Rebalance
        while (x->m_priority > x->parent()->m_priority)
        {
            auto y = x->parent();

            x == y->lchild() ?
                y->rotate_right(header.parent()) :
                y->rotate_left(header.parent());
        }
    }

    treap_node* rebalance_for_erase(treap_node& header)
    {
        auto x = this;

        auto& [root, leftmost, rightmost] = header.m_link;

        treap_node* child = nullptr;
        treap_node* parent = nullptr;

        if (x->lchild() && x->rchild())
        {   
            // Replace x with child which priority is larger
            if (x->lchild()->m_priority < x->rchild()->m_priority)
            {
                x->rotate_right(root);
                return x->rebalance_for_erase(header);
            }
            else
            {
                x->rotate_left(root);
                return x->rebalance_for_erase(header);
            }
        }
        else if (x->lchild())
        {
            // Replace x with left child
            child = x->lchild();
            if (x == rightmost)
            {
                rightmost = child->maximum();
            }
        }
        else if (x->rchild())
        {
            // Replace x with right child
            child = x->rchild();
            if (x == leftmost)
            {
                leftmost = child->minimum();
            }
        }
        else
        {
            // x is leaf
            if (x == leftmost)
            {
                leftmost = x->parent();
            }
            if (x == rightmost)
            {
                rightmost = x->parent();
            }
        }

        if (child)
        {
            child->parent(x->parent());
        }
        if (x == root)
        {
            root = child;
        }
        else
        {
            x->parent()->lchild() == x
                ? x->parent()->lchild(child)
                : x->parent()->rchild(child);   
        }

        return this;
    }

};


} // namespace leviathan::collections
