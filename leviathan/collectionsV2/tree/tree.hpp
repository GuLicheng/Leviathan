#pragma once

#include "../common.hpp"
#include "../associative_container_interface.hpp"
#include "tree_drawer.hpp"

namespace leviathan::collections
{ 

template <typename KeyValue,
	typename Compare,
	typename Allocator,
	bool UniqueKey, typename NodeType>
class tree : public row_drawer, 
		     public reversible_container_interface, 
			 public associative_container_insertion_interface,
			 public associative_container_lookup_interface<UniqueKey>
{
public:

	using key_value = KeyValue;
	using value_type = typename KeyValue::value_type;
	using key_type = typename KeyValue::key_type;
	using size_type = std::size_t;
	using allocator_type = Allocator;
	using difference_type = std::ptrdiff_t;
	using key_compare = Compare;
	using value_compare = Compare;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = std::allocator_traits<Allocator>::pointer;
	using const_pointer = std::allocator_traits<Allocator>::const_pointer;

	struct tree_node : NodeType
	{
		value_type m_val;

		value_type* value_ptr()
		{
			return std::addressof(m_val);
		}

		const value_type* value_ptr() const
		{
			return std::addressof(m_val);
		}

		NodeType* base()
		{
			return static_cast<NodeType*>(this);
		}

		const NodeType* base() const
		{
			return static_cast<const NodeType*>(this);
		}
	};

private:

	using node_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<tree_node>;
	using node_alloc_traits = std::allocator_traits<node_allocator>;

	static constexpr bool IsTransparent = detail::transparent<Compare>;

	template <typename U> using key_arg_t = detail::key_arg<IsTransparent, U, key_type>;

	static constexpr bool IsNothrowMoveConstruct =
		std::is_nothrow_move_constructible_v<Compare>
		&& typename node_alloc_traits::is_always_equal();

	static constexpr bool IsNothrowMoveAssign =
		std::is_nothrow_move_assignable_v<Compare>
		&& typename node_alloc_traits::is_always_equal();

	static constexpr bool IsNothrowSwap =
		std::is_nothrow_swappable_v<Compare>
		&& typename node_alloc_traits::is_always_equal();

	struct tree_iterator : postfix_increment_and_decrement_operation, arrow_operation
	{
		using link_type = NodeType*;
		using value_type = value_type;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::bidirectional_iterator_tag;
		// for set, reference should be const value_type&
		// for map, reference is just value_type&
		using reference = std::conditional_t<std::is_same_v<key_type, value_type>, const value_type&, value_type&>;

		link_type m_ptr;

		constexpr tree_iterator() = default;
		constexpr tree_iterator(const tree_iterator&) = default;

		constexpr tree_iterator(link_type ptr) : m_ptr(ptr) { }

		constexpr tree_iterator& operator++()
		{
			// if node is header/end/sentinel, we simply make it cycle
			m_ptr = m_ptr->is_header() ? m_ptr->lchild() : m_ptr->increment();
			return *this;
		}

		using postfix_increment_and_decrement_operation::operator++;

		constexpr tree_iterator& operator--()
		{
			m_ptr = m_ptr->is_header() ? m_ptr->rchild() : m_ptr->decrement();
			return *this;
		}

		using postfix_increment_and_decrement_operation::operator--;

		constexpr reference operator*(this tree_iterator it) 
		{
			return *(static_cast<tree_node*>(it.m_ptr)->value_ptr());
		}

		friend constexpr bool operator==(tree_iterator lhs, tree_iterator rhs) 
		{
			return lhs.m_ptr == rhs.m_ptr;
		}
	};

public:

	using iterator = tree_iterator;
	using const_iterator = std::const_iterator<iterator>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:

	tree() : tree(Compare(), Allocator()) { }

	explicit tree(const Compare& compare, const Allocator& allocator)
		: m_cmp(compare), m_alloc(allocator), m_size(0)
	{
		header()->as_empty_tree_header();
	}

	~tree()
	{
		clear();
	}

	void clear()
	{
		reset();
	}

	iterator begin()
	{
		return iterator(header()->lchild());
	}

	const_iterator begin() const
	{
		return iterator(header()->lchild());
	}

	iterator end()
	{
		return iterator(header());
	}

	const_iterator end() const
	{
		return iterator(header());
	}

	size_type size() const
	{
		return m_size;
	}

	bool empty() const
	{
		return size() == 0;
	}

	allocator_type get_allocator() const
	{
		return allocator_type(m_alloc);
	}

	size_type max_size() const
	{
		return node_alloc_traits::max_size(m_alloc);
	}

	key_compare key_comp() const
	{
		return m_cmp;
	}

	value_compare value_comp() const
	{
		return m_cmp;
	}

	// Modifiers
	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args)
	{
		using namespace leviathan::collections;
		if constexpr (detail::emplace_helper<value_type, Args...>::value)
		{
			return insert_unique((Args&&)args...);
		}
		else
		{
			return emplace_unique((Args&&)args...);
		}
	}

	template <typename... Args>
	std::pair<iterator, bool> emplace_hint(const_iterator pos, Args&&... args)
	{
		return emplace((Args&&) args...);
	}

	iterator erase(const_iterator pos) 
		requires (!std::same_as<iterator, const_iterator>)
	{
		return erase(pos.base());
	}

	iterator erase(iterator pos) 
	{
		auto ret = std::next(pos);
		erase_by_node(pos.m_ptr);
		return ret;
	}

	iterator erase(iterator first, iterator last)
	{
		if (first == begin() && last == end()) 
		{
			clear();
		}
		else
		{
			for (; first != last; first = erase(first));
		}
		return last;
	}

	iterator erase(const_iterator first, const_iterator last) 
		requires (!std::same_as<iterator, const_iterator>)
	{ 
		return erase(first.base(), last.base()); 
	}

	size_type erase(const key_type &x)
	{
		return erase_by_key(x);
	}

	// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2077r3.html
	template <typename K> requires (IsTransparent)
	size_type erase(K&& x) 
	{ 
		return erase_by_key(x); 
	}

	template <typename K = key_type>
	iterator lower_bound(const key_arg_t<K> &x)
	{
		return lower_bound_impl(x);
	}

	template <typename K = key_type>
	const_iterator lower_bound(const key_arg_t<K> &x) const
	{
		return const_cast<tree &>(*this).lower_bound(x);
	}

	NodeType* header() 
	{ 
		return &m_header; 
	}
	
	const NodeType* header() const 
	{ 
		return &m_header; 
	}

	tree_node *root()
	{
		return static_cast<tree_node*>
			(header()->parent());
	}

	const tree_node *root() const
	{
		return static_cast<const tree_node*>
			(header()->parent());
	}

private:

	using link_type = tree_node*;
	using const_link_type = const tree_node*;
	using base_ptr = NodeType*;
	using const_base_ptr = const NodeType*;

	template <typename K>
	iterator find_impl(const K& k)
	{
		iterator j = lower_bound_impl(k);
		return (j == end() || m_cmp(k, KeyValue()(*j))) ? end() : j;
	}

	void erase_by_node(base_ptr x)
	{      
		// auto y = NodeType::rebalance_for_erase(x, m_header);
		auto y = x->rebalance_for_erase(m_header);
		destroy_node(static_cast<link_type>(y));
		m_size--;
	}

	template <typename K>
	size_type erase_by_key(const K& x)
	{
		iterator node = this->find_impl(x);
		if (node != end())
		{
			erase_by_node(node.m_ptr);
			return 1;
		}
		return 0;
	}

	template <typename K>
	iterator lower_bound_impl(const K& k)
	{
		base_ptr y = &m_header, x = header()->parent();
		while (x)
			if (!m_cmp(keys(x), k))
			{
				// y = x, x = NodeType::left(x);
				y = x, x = x->lchild();
			}
			else
			{
				// x = NodeType::right(x);
				x = x->rchild();
			}
		return { y };
	}

	void dfs_deconstruct(base_ptr p)
	{
		if (p)
		{
			dfs_deconstruct(p->lchild());
			dfs_deconstruct(p->rchild());
			link_type node = static_cast<link_type>(p);
			drop_node(node);
		}
	}

	void reset()
	{
		dfs_deconstruct(header()->parent());
		header()->as_empty_tree_header();
		m_size = 0;
	}

	template <typename Arg>
	std::pair<iterator, bool> insert_unique(Arg&& v)
	{
		auto [x, p] = get_insert_unique_pos(KeyValue()(v));
		if (p)
		{
			return { insert_value(x, p, (Arg&&)v), true };
		}
		return { x, false };
	}

	template <typename... Args>
	std::pair<iterator, bool>
		emplace_unique(Args&& ...args)
	{
		link_type z = create_node((Args&&)args...);
		auto [x, p] = get_insert_unique_pos(keys(z));
		if (p)
			return { insert_node(x, p, z), true };
		drop_node(z);
		return { x, false };
	}

	iterator insert_node(base_ptr x, base_ptr p, link_type z)
	{
		bool insert_left = (x != 0 || p == &m_header || m_cmp(keys(z), keys(p)));
		//NodeType::insert_and_rebalance(insert_left, z, p, m_header);
		z->insert_and_rebalance(insert_left, p, m_header);
		++m_size;
		return iterator(z);
	}

	template<typename Arg>
	iterator insert_value(base_ptr x, base_ptr p, Arg&& v)
	{
		bool insert_left = (x != 0 || p == &m_header
			|| m_cmp(KeyValue()(v), keys(p)));

		link_type z = create_node((Arg&&)v);

		//NodeType::insert_and_rebalance(insert_left, z, p, m_header);
		z->insert_and_rebalance(insert_left, p, m_header);
		++m_size;
		return iterator(z);
	}

	std::pair<base_ptr, base_ptr> 
		get_insert_unique_pos(const key_type& k)
	{
		base_ptr y = &m_header, x = header()->parent();
		bool comp = true;

		while (x)
		{
			y = x;
			comp = m_cmp(k, keys(x));
			x = comp ? x->lchild() : x->rchild();
		}

		iterator j{ y };

		if (comp)
		{
			if (j == begin())
			{
				return { x, y };
			}
			else
			{
				--j;
			}
		}

		if (m_cmp(KeyValue()(*j), k))
		{
			return { x, y };
		}

		return { j.m_ptr, nullptr };
	}

	static const key_type& keys(const_link_type x)
	{
		return KeyValue()(*x->value_ptr());
	}

	static const key_type& keys(const_base_ptr x)
	{
		return keys(static_cast<const_link_type>(x));
	}

	link_type get_node()
	{
		return node_alloc_traits::allocate(m_alloc, 1);
	}

	void put_node(link_type p)
	{
		node_alloc_traits::deallocate(m_alloc, p, 1);
	}

	template <typename... Args>
	void construct_node(link_type node, Args&&... args)
	{
		try
		{
			//NodeType::init(node);
			node->init();
			node_alloc_traits::construct(m_alloc, node->value_ptr(), (Args&&)args...);
		}
		catch (...)
		{
			node_alloc_traits::destroy(m_alloc, node->value_ptr());
			put_node(node);
			throw;
		}
	}

	template <typename... Args>
	link_type create_node(Args&&... args)
	{
		link_type tmp = get_node();
		construct_node(tmp, (Args&&)args...);
		return tmp;
	}

	void destroy_node(link_type p)
	{
		node_alloc_traits::destroy(m_alloc, p->value_ptr());
	}

	void drop_node(link_type p)
	{
		destroy_node(p);
		put_node(p);
	}

	[[no_unique_address]] Compare m_cmp;
	[[no_unique_address]] node_allocator m_alloc;
	NodeType m_header;
	size_type m_size;
};

}