/*
	Since each node will occupy memory allocated by allocator,
	the node will always be lvalue.
*/
#pragma once

#include <type_traits>

#include <assert.h>

namespace leviathan::collections
{

struct basic_tree_node_operation
{
	template <typename Node>
	Node* parent(this Node& self)
	{
		return self.m_nodes[0];
	}

	template <typename Node>
	Node* lchild(this Node& self)
	{
		return self.m_nodes[1];
	}

	template <typename Node>
	Node* rchild(this Node& self)
	{
		return self.m_nodes[2];
	}

	template <typename Node>
	void parent(this Node& self, std::type_identity_t<Node>* node)
	{
		self.m_nodes[0] = node;
	}

	template <typename Node>
	void lchild(this Node& self, std::type_identity_t<Node>* node)
	{
		self.m_nodes[1] = node;
	}

	template <typename Node>
	void rchild(this Node& self, std::type_identity_t<Node>* node)
	{
		self.m_nodes[2] = node;
	}
};

struct binary_node_operation : basic_tree_node_operation
{
	// Return rightmost of binary node.
	template <typename Node>
	constexpr Node* maximum(this Node& self)
	{
		auto x = std::addressof(self);
		for (; x->rchild(); x = x->rchild());
		return x;
	}

	// Return leftmost of binary node.
	template <typename Node>
	constexpr Node* minimum(this Node& self)
	{
		auto x = std::addressof(self);
		for (; x->lchild(); x = x->lchild());
		return x;
	}

	// Return prev position of binary node.
	template <typename Node>
	constexpr Node* decrement(this Node& self)
	{
		auto x = std::addressof(self);

		if (x->lchild())
		{
			return x->lchild()->maximum();
		}
		else
		{
			auto y = x->parent();

			while (x == y->lchild())
			{
				x = y;
				y = y->parent();
			}

			if (x->lchild() != y)
			{
				x = y;
			}

			return x;
		}
	}

	// Return next position of binary node.
	template <typename Node>
	constexpr Node* increment(this Node& self)
	{
		auto x = std::addressof(self);

		if (x->rchild())
		{
			return x->child()->minimum();
		}
		else
		{
			auto y = x->parent();

			while (x == y->rchild())
			{
				x = y;
				y = y->parent();
			}

			if (x->rchild() != y)
			{
				x = y;
			}

			return x;
		}
	}
};

}
