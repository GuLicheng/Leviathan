// https://medium.com/carpanese/a-visual-introduction-to-treap-data-structure-part-1-6196d6cc12ee
#pragma once

#include "tree_node_operation.hpp"

#include <random>

namespace leviathan::collections
{

template <typename RandomEngine = std::mt19937, typename SeedGenerator = std::random_device>
struct treap_node : binary_node_operation
{
    static_assert(std::is_unsigned_v<typename RandomEngine::result_type>);

    // All nodes share one random generator
    inline static RandomEngine random = RandomEngine(SeedGenerator()());

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
        assert(this->m_priority != RandomEngine::max() && "The node should not be header");
        this->insert_node_and_update_header(insert_left, p, header);

        // Rebalance
        auto x = this;
        
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

        while (x->lchild() || x->rchild())
        {
            if (!x->rchild() || (x->lchild() && x->lchild()->m_priority > x->rchild()->m_priority))
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

        return x;
    }
};

using default_treap_node = treap_node<>;

template <size_t N>
struct always
{
    static size_t operator()()
    {
        return N;
    }
};

using debug_treap_node = treap_node<std::mt19937, always<0>>;

} // namespace leviathan::collections
