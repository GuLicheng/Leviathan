#pragma once

#include <leviathan/collections/core/node_interface.hpp>

namespace cpp::collections::intrusive
{

struct avl_hook : tree_node_interface
{
    static constexpr int balance_factor = 2;

    avl_hook* links[3];
    
    int balance;

    constexpr void as_sentinel() 
    { 
        // Sentinel nodes have a balance of -1
        balance = -1; 
    }

    constexpr bool is_sentinel() const
    {
        return balance == -1;
    }

    constexpr void link(bool insert_left, avl_hook* p, avl_hook& header)
    {
        // assert(this->balance == 1 && "The node should be a leaf node");
        this->insert_node_and_update_header(insert_left, p, header);
        balance = 1;

        // Rebalance
        for (auto x = this->parent(); x != &header; x = x->parent())
        {
            int lh = height(x->lchild());
            int rh = height(x->rchild());
            int h = std::max(lh, rh) + 1;

            if (x->balance == h) 
            {
                break;
            }
            
            x->balance = h;

            int diff = lh - rh;

            if (diff <= -2)
            {
                x->avl_tree_fix_l(&header);
            }
            else if (diff >= 2)
            {
                x->avl_tree_fix_r(&header);
            }
        }
    }

    constexpr void unlink(avl_hook& header)
    {
        auto [successor, child, child_parent] = this->replace_node_with_successor(header);

        if (successor != this)
        {
            successor->balance = balance;
        }
        else
        {
            child_parent = successor->parent();
        }

        child_parent->avl_tree_rebalance_erase(&header);
    }

private:

    static constexpr int height(const avl_hook* node)
    {
        return node ? node->balance : 0;
    }

    constexpr void update_balance()
    {
        balance = 1 + std::max(height(lchild()), height(rchild()));
    }

    constexpr avl_hook* avl_tree_fix_l(avl_hook* header)
    {
        auto x = this;
        auto r = x->rchild();
        int lh0 = height(r->lchild());
        int rh0 = height(r->rchild());

        if (lh0 > rh0)
        {
            r->rotate_right(header->parent());
            r->update_balance();
            r->parent()->update_balance();
        }

        x->rotate_left(header->parent());
        x->update_balance();
        x->parent()->update_balance();
        return x->parent();
    }

    constexpr avl_hook* avl_tree_fix_r(avl_hook* header)
    {
        auto x = this;
        auto l = x->lchild();
        int lh0 = height(l->lchild());
        int rh0 = height(l->rchild());

        if (lh0 < rh0)
        {
            l->rotate_left(header->parent());
            l->update_balance();
            l->parent()->update_balance();
        }

        x->rotate_right(header->parent());
        x->update_balance();
        x->parent()->update_balance();
        return x->parent();
    }

    constexpr void avl_tree_rebalance_erase(avl_hook* header)
    {
        auto x = this;

        for (; x != header; x = x->parent())
        {
            int lh = height(x->lchild());
            int rh = height(x->rchild());
            int diff = lh - rh;
            int h = std::max(lh, rh) + 1;
            
            if (x->balance != h)
            {
                x->balance = h;
            }
            else if (-1 <= diff && diff <= 1) 
            {
                break;
            }

            if (diff <= -2)
            {
                x = x->avl_tree_fix_l(header);
            }
            else if (diff >= 2)
            {
                x = x->avl_tree_fix_r(header);
            }
        }
    }

};

namespace detail
{

class avl_checker
{
    bool m_result = true;

    constexpr void check(bool condition)
    {
        if (!condition) m_result = false;
    }

    constexpr static int height(const avl_hook* root) 
    {
        return root ? root->balance : 0;
    }

    constexpr void traverse(const avl_hook* node) 
    {
        if (node)
        {
            traverse(node->lchild());
            traverse(node->rchild());

            int lh = height(node->lchild());
            int rh = height(node->rchild());
            int diff = lh - rh;

            check(-1 <= diff && diff <= 1);
            check(lh >= 0 && rh >= 0);
            check(node->balance == std::max(lh, rh) + 1);
        }
    }

public:

    constexpr bool operator()(const avl_hook* p) 
    {
        m_result = true;
        traverse(p);
        return m_result;
    }
};

class avl_mermaid_render
{
    template <typename ToString>
    static constexpr void render_mermaid_impl(const avl_hook* node, std::string& result, ToString to_string)
    {
        if (!node)
        {
            return;
        }

        auto addr = reinterpret_cast<uintptr_t>(node);
        result += std::format("{}[\"{}\"]\n", addr, to_string(node));

        if (node->lchild())
        {
            auto left = reinterpret_cast<uintptr_t>(node->lchild());
            result += std::format("{} --> {}\n", addr, left);
        }
        
        if (node->rchild())
        {
            auto right = reinterpret_cast<uintptr_t>(node->rchild());
            result += std::format("{} --> {}\n", addr, right);
        }
        
        render_mermaid_impl(node->lchild(), result, to_string);
        render_mermaid_impl(node->rchild(), result, to_string);
    }

public:

    template <typename ToString>
    static constexpr std::string operator()(const avl_hook* root, ToString to_string)
    {
        if (!root)
            return "```mermaid\n"
                   "flowchart TD\n"
                   "    classDef nil fill:#fff,stroke:#333,stroke-dasharray:4 4\n"
                   "    N[∅]:::nil\n"
                   "```";

        std::string result = "```mermaid\nflowchart TD\n";
        render_mermaid_impl(root, result, to_string);
        result += "```";
        return result;
    }

};

}  // namespace detail

}  // namespace cpp::collections::intrusive
