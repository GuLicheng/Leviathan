#pragma once

#include "tree_node_operation.hpp"

namespace leviathan::collections
{

struct binary_node : binary_node_operation
{
	// Nodes
	binary_node* m_nodes[3];
	
	// True for header and false for others
	bool m_sentinel;

	// Initialize node without value field after calling allocate
	void init()
	{
		this->parent(nullptr);
		this->lchild(nullptr);
		this->rchild(nullptr);
		m_sentinel = false;
	}

	// Reset header for an empty tree 
	void as_empty_tree_header()
	{
		this->parent(nullptr);
		this->lchild(this);
		this->rchild(this);
		m_sentinel = true;
	}

	bool is_header() const
	{
		return m_sentinel;
	}

	void insert_and_rebalance(bool insert_left, binary_node* p, binary_node& header)
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
	}

	binary_node* rebalance_for_erase(binary_node& header)
	{
		auto x = this;

		binary_node*& root = header.m_nodes[0];
		binary_node*& leftmost = header.m_nodes[1];
		binary_node*& rightmost = header.m_nodes[2];

		binary_node* child = nullptr;
		binary_node* parent = nullptr; // for rebalance

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
			// parent = x->parent();
		}
		//return z;
		return this;
	}

	void clone(const binary_node* x)
	{ }
};

}

