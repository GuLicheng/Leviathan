#pragma once

#include "tree_node_operation.hpp"

#include <algorithm>

namespace leviathan::collections
{
    
struct avl_node : basic_tree_node_operation, binary_node_operation
{
    static constexpr int balance_factor = 2;

    // Nodes
    avl_node* m_nodes[3];

    // Height of current node, -1 for header, 1 for leaf and 0 for nullptr
    int m_height;

	// Initialize node without value field after calling allocate
	void init()
	{
		this->parent(nullptr);
		this->lchild(nullptr);
		this->rchild(nullptr);
		m_height = 1;
	}

	// Reset header for an empty tree 
	void as_empty_tree_header()
	{
		this->parent(nullptr);
		this->lchild(this);
		this->rchild(this);
		m_height = -1;
	}

    static int height(const avl_node* node)
    {
        return node ? node->m_height : 0;
    }

    void update_height()
    {
        int lh = height(lchild());
        int rh = height(rchild());
        m_height = std::max(lh, rh) + 1;
    }

    void clone(const avl_node* x)
    {
        m_height = x->m_height;
    }

    bool is_header() const
    {
        return m_height == -1;
    }

    void erase_node(avl_node* header)
    {
        auto x = this;

        avl_node*& root = header->m_nodes[0];
        avl_node*& leftmost = header->m_nodes[1];
        avl_node*& rightmost = header->m_nodes[2];

        avl_node* child = nullptr;
        avl_node* parent = nullptr; // for rebalance

        if (x->lchild() && x->rchild())
        {
            auto successor = x->rchild()->minimum();
            child = successor->rchild();
            parent = successor->parent();

            if (child)
            {
                child->parent(parent);
            }

            successor->parent()->lchild() == successor 
                ? successor->parent()->lchild(child)
                : successor->parent()->rchild(child);

			if (successor->parent() == x)
			{
				parent = successor;
			}
            
			successor->lchild(x->lchild());
			successor->rchild(x->rchild());
			successor->parent(x->parent());
            successor->m_height = x->m_height;
        
			if (x == root)
			{
				root = successor;
			}
			else
			{
				x->parent()->lchild() == x 
					? x->parent()->lchild(successor)
					: x->parent()->rchild(successor);
			}

			x->lchild()->parent(successor);

			if (x->rchild())
			{
				x->rchild()->parent(successor);
			}
        }
        else
        {
            // update leftmost or rightmost
			if (!x->lchild() && !x->rchild())
			{
				// leaf, such as just one root
				if (x == leftmost)
				{
					leftmost = x->parent();
				}
				if (x == rightmost)
				{
					rightmost = x->parent();
				}
			}
			else if (x->lchild())
			{
				// only left child
				child = x->lchild();
				if (x == rightmost)
				{
					rightmost = child->maximum();
				}
			}
			else
			{
				// only right child
				child = x->rchild();
				if (x == leftmost)
				{
					leftmost = child->minimum();
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

            parent = x->parent();
        }
        parent->avl_tree_rebalance_erase(header);
    }

    void avl_tree_fix_l(avl_node* header)
    {
        auto x = this;
        auto r = x->rchild();
        int lh0 = height(r->lchild());
        int rh0 = height(r->rchild());

        if (lh0 > rh0)
        {
            r->rotate_right(header->m_nodes[0]);
            r->update_height();
            r->parent()->update_height();
        }

        x->rotate_left(header->m_nodes[0]);
        x->update_height();
        x->parent()->update_height();
    }

    void avl_tree_fix_r(avl_node* header)
    {
        auto x = this;
        auto l = x->lchild();
        int lh0 = height(l->lchild());
        int rh0 = height(l->rchild());

        if (lh0 < rh0)
        {
            l->rotate_left(header->m_nodes[0]);
            l->update_height();
            l->parent()->update_height();
        }

        x->rotate_right(header->m_nodes[0]);
        x->update_height();
        x->parent()->update_height();
    }

    void avl_tree_rebalance_insert(avl_node* header)
    {
        auto x = this;

        for (x = x->parent(); x != header; x = x->parent())
        {
            int lh = height(x->lchild());
            int rh = height(x->rchild());
            int h = std::max(lh, rh) + 1;

            if (height(x) == h) 
            {
                break;
            }
            
            x->m_height = h;

            int diff = lh - rh;

            if (diff <= -2)
            {
                x->avl_tree_fix_l(header);
            }
            else if (diff >= 2)
            {
                x->avl_tree_fix_r(header);
            }
        }
    }

    void avl_tree_rebalance_erase(avl_node* header)
    {
        auto x = this;

        for (; x != header; x = x->parent())
        {
            int lh = height(x->lchild());
            int rh = height(x->rchild());
            int h = std::max(lh, rh) + 1;
            x->m_height = h;
            int diff = lh - rh;
            
            if (x->m_height != h)
            {
                x->m_height = h;
            }
            else if (-1 <= diff && diff <= 1) 
            {
                break;
            }

            if (diff <= -2)
            {
                x->avl_tree_fix_l(header);
            }
            else 
            {
                x->avl_tree_fix_r(header);
            }
        }
    }

    void insert_and_rebalance(bool insert_left,
                                    avl_node* p,
                                    avl_node& header)
    {
        auto x = this;

        x->parent(p);
        x->lchild(nullptr);
        x->rchild(nullptr);
        x->m_height = 1;

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

        // rebalance
        x->avl_tree_rebalance_insert(&header);
    }

    avl_node* rebalance_for_erase(avl_node& header)
    {
        auto z = this;
        z->erase_node(&header);
        return z;
    }
    
};

} // namespace leviathan::collections

