#pragma once
// https://leetcode-cn.com/problems/design-skiplist/

#ifndef __SKIPLIST_HPP__
#define __SKIPLIST_HPP__

#include <lv_cpp/meta/meta.hpp>

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <random>
#include <array>
#include <iterator>
#include <memory_resource>
#include <type_traits>


namespace leviathan::collections
{
	template <typename T, typename U>
	struct insert_emplace_selector : std::conditional_t
		<
		std::is_same_v<std::remove_cvref_t<U>, std::remove_cvref_t<T>>,
		std::true_type, // insert
		std::false_type  // emplace
		> { };

	template <typename T>
	struct indentity
	{
		using iter_value_type = T;
		using key_type = T;
		template <typename Compare, typename Lhs, typename Rhs>
		constexpr static bool compare(const Compare& cmp, const Lhs& lhs, const Rhs& rhs)
		noexcept(noexcept(cmp(lhs, rhs)))
		{
			return cmp(lhs, rhs);
		}

		template <typename U>
		constexpr static auto& get_key(const U& u) noexcept
		{
			return u;
		}

	};

	template <typename T>
	struct select1st
	{
	private:
		using _1st = std::tuple_element_t<0, T>;
		using _2nd = std::tuple_element_t<1, T>;
	public:
		using iter_value_type = std::pair<std::add_const_t<_1st>, _2nd>;
		using key_type = _1st;

		template <typename Compare, typename Lhs, typename Rhs>
		constexpr static bool compare(const Compare& cmp, const Lhs& lhs, const Rhs& rhs)
		noexcept(noexcept(cmp(lhs.first, rhs)))
		{
			// lhs is pair already inserted, but rhs may just a key_type
			return cmp(lhs.first, rhs);
		}

		template <typename U>
		constexpr static auto& get_key(const U& u) noexcept
		{
			return u.first;
		}

	};

	template <typename Key, typename Compare = std::less<Key>, typename Allocator = std::allocator<Key>, typename KeyTraits = indentity<Key>>
	class skip_list
	{
	public:

		constexpr static int MAXLEVEL = 32;
		inline static std::random_device rd;
		constexpr static unsigned int p = std::random_device::max() / 4;

		static int get_level()
		{
			int level = 1;
			for (; rd() < p; ++level);
			return std::min(MAXLEVEL, level);
		}

		template <typename Derived>
		struct header
		{
			Derived* m_prev;
			std::vector<Derived*> m_next;
			header(Derived* prev = nullptr, std::size_t num = MAXLEVEL) noexcept
				: m_prev{ prev }, m_next{ num, nullptr } { } // make it noexcept

			header(const header&) = default;
			header(header&&) noexcept = default;
			header& operator=(const header&) = default;
			header& operator=(header&&) noexcept = default;

			auto derived_ptr() noexcept
			{
				return static_cast<Derived*>(this);
			}

			auto derived_ptr() const noexcept
			{
				return static_cast<const Derived*>(this);
			}

		};

		struct skip_node : public header<skip_node>
		{
			typename KeyTraits::iter_value_type m_data;
			using base = header<skip_node>;
			// skip_node() { }

			skip_node(const skip_node&) = default;
			skip_node(skip_node&&) 
				noexcept(noexcept(std::is_nothrow_move_constructible_v<typename KeyTraits::iter_value_type>)) = default;
			skip_node& operator=(const skip_node&) = default;
			skip_node& operator=(skip_node&&) 
				noexcept(noexcept(std::is_nothrow_move_assignable_v<typename KeyTraits::iter_value_type>)) = default;

			skip_node(const Key& x, std::size_t nextNum)
				: base(this, nextNum), m_data(x) { }
			skip_node(Key&& x, std::size_t nextNum)
				: base(this, nextNum), m_data(std::move(x)) { }

			skip_node(std::size_t nextNum)
			{
				skip_node(typename KeyTraits::iter_value_type{ }, nextNum);
			}


		};
		using key_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<skip_node>;

	public:
		template <bool Const>
		struct skip_list_iterator;

		using const_iterator = skip_list_iterator<true>;
		using iterator = skip_list_iterator<false>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = reverse_iterator;

		using allocator_type = Allocator;
		using key_type = typename KeyTraits::key_type;
		using value_type = typename KeyTraits::iter_value_type; // add const for map
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using key_compare = Compare;
	private:
		constexpr static bool isNoexceptMoveAssign 
			= std::is_nothrow_move_assignable_v<value_type>
			&& std::is_nothrow_move_assignable_v<key_compare>
			&& std::is_nothrow_move_assignable_v<key_allocator_type>;
		constexpr static bool isNoexceptMoveConstruct 
			= std::is_nothrow_move_constructible_v<value_type> 
			&& std::is_nothrow_move_constructible_v<key_compare>
			&& std::is_nothrow_move_constructible_v<key_allocator_type>
			&& std::allocator_traits<key_allocator_type>::propagate_on_container_move_assignment::value;

	public:
		skip_list() noexcept(noexcept(std::is_nothrow_default_constructible_v<key_allocator_type>
			&& std::is_nothrow_default_constructible_v<Compare>))
			: m_cmp{ }, m_alloc{ }, m_size{ 0 }, m_header{ }, m_level{ 1 }
		{
		}

		skip_list(const allocator_type& alloc)
			: m_cmp{ }, m_alloc{ alloc }, m_size{ 0 }, m_header{ }, m_level{ 1 } 
		{
		}

		~skip_list() noexcept
		{
			clear();
		}
		// https://github.com/CppCon/CppCon2017 - How to Write a Custom Allocator 
		// https://github.com/CppCon/CppCon2017/blob/master/Tutorials/How%20to%20Write%20a%20Custom%20Allocator/How%20to%20Write%20a%20Custom%20Allocator%20-%20Bob%20Steagall%20-%20CppCon%202017.pdf
		skip_list(const skip_list& rhs)
			: m_cmp{ rhs.m_cmp }, m_size{ }, m_header{ }, m_level{ 1 }
		{
			// may use impl to warp all flied but alloc
			// traits::select_on_container_copy_construction(rhs.m_alloc)
			this->m_alloc = std::allocator_traits<key_allocator_type>::select_on_container_copy_construction(rhs.m_alloc);
			// this->assign_from(rhs.cbegin(), rhs.cend());
			try 
			{
				assign_from(rhs.cbegin(), rhs.cend());
			}
			catch (...)
			{
				clear();
				throw; // rethrow exception
			}
		}

		skip_list(skip_list&& rhs) 
		noexcept(isNoexceptMoveConstruct)
			: m_cmp{ std::move(rhs.m_cmp) }, m_alloc{ std::move(rhs.m_alloc) }, m_size{ rhs.m_size }, m_header{ std::move(rhs.m_header) }, m_level{ rhs.m_level }
		{
		}

		skip_list& operator=(const skip_list& rhs)
		{
			if (std::addressof(rhs) != this)
			{
				// std::true_type
				if constexpr (typename std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment())
				{
					if (this->m_alloc != rhs.m_alloc)
					{
						// this->clear_and_deallocate_memory()
						clear_and_deallocate_memory();
					}
					this->m_alloc = rhs.m_alloc;
				}
				// this->assign_from(rhs.begin(), rhs.end());
				try
				{
					assign_from(rhs.cbegin(), rhs.cend());
				}
				catch (...)
				{
					clear();
					throw;
				}
			}
			return *this;
		}
		// https://stackoverflow.com/questions/27471053/example-usage-of-propagate-on-container-move-assignment
		skip_list& operator=(skip_list&& rhs) 
		noexcept(isNoexceptMoveAssign)
		{
			if (this != std::addressof(rhs))
			{
				if constexpr (typename std::allocator_traits<key_allocator_type>::propagate_on_container_move_assignment())
				{
					// this->clear_and_deallocate_memory
					// move alloc and impl
					clear_and_deallocate_memory();
					this->m_alloc = std::move(rhs.m_alloc);
					this->m_header = std::move(rhs.m_header);
					this->m_size = rhs.m_size;
					this->m_cmp = std::move(rhs.m_cmp);
					this->m_level = rhs.m_level;
				}
				else if (typename std::allocator_traits<key_allocator_type>::is_always_equal() || this->m_alloc == rhs.m_alloc)
				{
					// this->clear_and_deallocate_memory()
					// this->impl = move(rhs.impl)
					clear_and_deallocate_memory();
					this->m_header = std::move(rhs.m_header);
					this->m_size = rhs.m_size;
					this->m_cmp = std::move(rhs.m_cmp);
					this->m_level = rhs.m_level;
				}
				else
				{
					// this->assign(move_iter(rhs.begin()), move_iter(rhs.end()));
					assign_from(std::make_move_iterator(rhs.begin()), std::make_move_iterator(rhs.end()));
				}
			}
			return *this;
		}

		void swap(skip_list& rhs)
		{
			if (this != std::addressof(rhs))
			{
				if constexpr (typename std::allocator_traits<key_allocator_type>::propagate_on_container_swap())
				{
					// std::swap(impl and alloc)
					std::swap(this->m_cmp, rhs.m_cmp);
					std::swap(this->m_size, rhs.m_size);
					std::swap(this->m_header, rhs.m_header);
					std::swap(this->m_level, rhs.m_level);
					std::swap(this->m_alloc, rhs.m_alloc);
				}
				else if (typename std::allocator_traits<key_allocator_type>::is_always_equal()
					|| this->m_alloc == rhs.m_alloc)
				{
					// std::swap(impl)
					std::swap(this->m_cmp, rhs.m_cmp);
					std::swap(this->m_size, rhs.m_size);
					std::swap(this->m_header, rhs.m_header);
					std::swap(this->m_level, rhs.m_level);
				}
				else
				{
					// Undefined Behaviour
					throw std::runtime_error("Undefined Behaviour");
				}
			}
		}

		void clear() noexcept
		{
			clear_and_deallocate_memory();
			this->m_size = 0;
			this->m_header.m_prev = nullptr;
			std::fill(this->m_header.m_next.begin(), this->m_header.m_next.end(), nullptr);
		}
		void show() const
		{
			auto cur = this->m_header.m_next[0];
			while (cur)
			{
				std::cout << cur->m_data << "(" << cur->m_next.size() << ")  ";
				cur = cur->m_next[0];
			}
		}

		iterator begin() noexcept
		{
			return iterator(this->m_header.m_next[0], this);
		}

		iterator end() noexcept
		{
			return iterator(nullptr, this);
		}

		const_iterator begin() const noexcept
		{
			return const_iterator(this->m_header.m_next[0], this);
		}

		const_iterator cbegin() const noexcept
		{
			return begin();
		}

		const_iterator end() const noexcept
		{
			return const_iterator(nullptr, this);
		}

		const_iterator cend() const noexcept
		{
			return end();
		}

		reverse_iterator rbegin() noexcept
		{
			return std::make_reverse_iterator(end());
		}

		reverse_iterator rend() noexcept
		{
			return std::make_reverse_iterator(begin());
		}

		const_reverse_iterator rbegin() const noexcept
		{
			return std::make_reverse_iterator(end());
		}

		const_reverse_iterator rend() const noexcept
		{
			return std::make_reverse_iterator(begin());
		}


		template <typename... Args>
		std::pair<iterator, bool> emplace(Args&&... args)
		{
			// check type -> emplace or insert
			// emplace will create node first and then try to insert
			// if args is value_type, emplace will call move or copy construct 
			// and cost more if node already exist
			if constexpr (sizeof...(Args) == 1)
			{
				if constexpr (insert_emplace_selector<Key, Args...>::value)
					return insert(std::forward<Args>(args)...);
				else
				{
					auto [node, exist] = emplace_unique(this->m_header.derived_ptr(), std::forward<Args>(args)...);
					return { iterator(node, this), exist };
				}
			}
			else
			{
				auto [node, exist] = emplace_unique(this->m_header.derived_ptr(), std::forward<Args>(args)...);
				return { iterator(node, this), exist };
			}
		}

		std::pair<iterator, bool> insert(const value_type& val)
		{
			auto [node, exist] = insert_unique(this->m_header.derived_ptr(), val);
			return { iterator(node, this), exist };
		}

		std::pair<iterator, bool> insert(value_type&& val)
		{
			auto [node, exist] = insert_unique(this->m_header.derived_ptr(), std::move(val));
			return { iterator(node, this), exist };
		}

		const_iterator lower_bound(const key_type& val) const
		{
			return const_iterator{ lower_bound_impl(this->m_header.derived_ptr(), val), this };
		}

		iterator lower_bound(const key_type& val)
		{
			return iterator{ lower_bound_impl(this->m_header.derived_ptr(), val), this };
		}

		const_iterator find(const key_type& val) const
		{
			auto [node, exist] = find_node(this->m_header.derived_ptr(), val);
			return const_iterator{ (exist ? node : nullptr), this };
		}

		iterator find(const key_type& val)
		{
			auto [node, exist] = find_node(this->m_header.derived_ptr(), val);
			return iterator{ (exist ? node : nullptr), this };
		}

		iterator erase(const key_type& val)
		{
			return iterator{ erase_node(this->m_header.derived_ptr(), val), this };
		}

		std::size_t size() const noexcept
		{
			return this->m_size;
		}

		bool empty() const noexcept
		{
			return this->m_size == 0;
		}

	private:
		[[no_unique_address]] Compare m_cmp;
		[[no_unique_address]] key_allocator_type m_alloc;
		std::size_t m_size;
		header<skip_node> m_header;
		int m_level;

		template <typename Iter, typename Sent>
		void assign_from(Iter first, Sent last)
		{
			for (auto iter = first; iter != last; ++iter)
				insert(*iter);
		}

		void clear_and_deallocate_memory()
		{
			// if (this->m_header.m_next.empty())
			// 	return;
			assert(this->m_header.m_next.size() > 0);
			auto cur = this->m_header.m_next[0];
			while (cur)
			{
				auto next = cur->m_next[0];
				destory_one_node(cur);
				cur = next;
			}
		}


		// return target node if succeed else prev position of target node for inserting
		template <bool CompletelyPrev>
		std::pair<skip_node*, bool>
			find_node_with_prev(skip_node* pos, const key_type& val, std::array<skip_node*, MAXLEVEL>& prev) const noexcept
		{
			skip_node* cur = pos;
			bool exist = false;
			for (int i = this->m_level - 1; i >= 0; --i)
			{
				// cur = pos;
				for (; cur->m_next[i] && KeyTraits::compare(this->m_cmp, cur->m_next[i]->m_data, val); cur = cur->m_next[i]);
				auto next_node = cur->m_next[i];
				if (next_node && KeyTraits::compare(std::equal_to<>(), next_node->m_data, val))
				{
					if constexpr (!CompletelyPrev)
						return { next_node, true }; // find it and do nothing
					else
						exist = true;
				}
				prev[i] = cur;
			}
			// cur is prev of node
			return { cur, exist };
		}

		void insert_after(skip_node* pos, skip_node* new_node, const std::array<skip_node*, MAXLEVEL>& prev) noexcept
		{
			// update prev
			new_node->m_prev = pos;
			if (pos->m_next[0])
				pos->m_next[0]->m_prev = new_node;
			// update next
			const int level = static_cast<int>(new_node->m_next.size());
			for (int i = 0; i < level; ++i)
			{
				if (i >= this->m_level)
					this->m_header.m_next[i] = new_node;
				else
				{
					new_node->m_next[i] = prev[i]->m_next[i];
					prev[i]->m_next[i] = new_node;
				}
			}
		}

		void update_header(skip_node* pos) noexcept
		{
			// check head
			if (!pos->m_prev)
				this->m_header.m_next[0] = pos;
			// check tail
			if (!pos->m_next[0])
				this->m_header.m_prev = pos;
		}

		// find node without prev
		std::pair<skip_node*, bool>
			find_node(skip_node* pos, const key_type& val) const
		{
			skip_node* cur = pos;
			for (int i = this->m_level; i >= 0; --i)
			{
				for (; cur->m_next[i] && KeyTraits::compare(this->m_cmp, cur->m_next[i]->m_data, val); cur = cur->m_next[i]);
				auto next_node = cur->m_next[i];
				if (next_node && KeyTraits::compare(std::equal_to<>(), next_node->m_data, val))
				{
					return { cur->m_next[i], true };
				}
			}
			return { cur, false };
		}

		template <typename U>
		std::pair<skip_node*, bool> insert_unique(skip_node* pos, U&& val)
		{
			std::array<skip_node*, MAXLEVEL> prev;
			prev.fill(nullptr);
			auto [cur, exist] = find_node_with_prev<false>(pos, KeyTraits::get_key(val), prev);
			if (exist)
				return { cur, false };

			// insert a new node
			auto level = get_level();
			auto new_node = create_one_node(level, std::forward<U>(val));
			insert_and_update_all_member(cur, new_node, prev, level);
			return { new_node, true };
		}

		// insert and emplace will always recept a value_type
		template <typename... Args>
		std::pair<skip_node*, bool> emplace_unique(skip_node* pos, Args&&... args)
		{
			auto level = get_level();
			auto new_node = create_one_node(level, std::forward<Args>(args)...);
			std::array<skip_node*, MAXLEVEL> prev;
			prev.fill(nullptr);
			auto [cur, exist] = find_node_with_prev<false>(pos, KeyTraits::get_key(new_node->m_data), prev);
			if (exist)
			{
				destory_one_node(new_node);
				return { cur, false };
			}
			insert_and_update_all_member(cur, new_node, prev, level);
			return { new_node, true };
		}

		void insert_and_update_all_member(skip_node* pos, skip_node* new_node, std::array<skip_node*, MAXLEVEL>& prev, int new_level)
		{
			insert_after(pos, new_node, prev);
			update_header(new_node);
			this->m_level = std::max(this->m_level, new_level);
			++this->m_size;
		}

		skip_node* erase_node(skip_node* pos, const key_type& val)
		{
			std::array<skip_node*, MAXLEVEL> prev;
			prev.fill(nullptr);
			auto [cur, exist] = find_node_with_prev<true>(pos, val, prev);
			if (!exist)
				return cur->m_next[0];

			skip_node* deleted_node = cur->m_next[0];
			if (deleted_node->m_next[0])
				deleted_node->m_next[0]->m_prev = cur;
			for (std::size_t i = 0; i < deleted_node->m_next.size(); ++i)
				prev[i]->m_next[i] = deleted_node->m_next[i];

			int new_level;
			for (new_level = this->m_level - 1; new_level >= 0 && this->m_header.m_next[new_level] == nullptr; --new_level);
			this->m_level = new_level + 1;

			// int new_level = m_level;
			// while (--new_level >= 0 && m_header.m_next[new_level] == nullptr)
			// 	m_level = new_level;

			destory_one_node(deleted_node);
			--this->m_size;
			return prev[0]->m_next[0];
		}

		skip_node* lower_bound_impl(skip_node* pos, const key_type& val) const
		{
			auto [cur, exist] = find_node(pos, val);
			return exist ? cur : cur->m_next[0];
		}

		void destory_one_node(skip_node* p) noexcept // assume destructor and deallocate is exception-safe
		{
			std::destroy_at(p);
			this->m_alloc.deallocate(p, sizeof(skip_node));
		}

		template <typename... Args>
		skip_node* create_one_node(int level, Args&&... args)
		{
			// return new skip_node(std::forward<Args>(args)..., level);
			// default allocator is must slower than new, I don't know why
			auto addr = this->m_alloc.allocate(sizeof(skip_node));
			assert(addr != nullptr); // if (!addr) throw std::bad_alloc { }
			std::construct_at(addr, std::forward<Args>(args)..., level);
			return addr;
		}

	};

	template <typename Key, typename Compare, typename Allocator, typename KeyTraits>
	template <bool Const>
	struct skip_list<Key, Compare, Allocator, KeyTraits>::skip_list_iterator
	{
		using value_type = typename KeyTraits::iter_value_type;
		using link_container_type = meta::maybe_const_t<Const, skip_list<Key, Compare, Allocator, KeyTraits>*>;
		using link_type = meta::maybe_const_t<Const, skip_node*>;

		using reference = meta::maybe_const_t<Const, value_type&>;
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;
		link_type m_ptr;
		link_container_type m_slist;

		constexpr skip_list_iterator() noexcept = default;
		constexpr skip_list_iterator(link_type ptr, link_container_type c) noexcept
			: m_ptr{ ptr }, m_slist{ c } { }

		template <bool IsConst, typename = std::enable_if_t<((Const == IsConst) || Const)>>
		constexpr skip_list_iterator(const skip_list_iterator<IsConst>& rhs) noexcept
			: m_ptr{ rhs.m_ptr }, m_slist{ rhs.m_slist } { }

		template <bool IsConst, typename = std::enable_if_t<((Const == IsConst) || Const)>>
		constexpr skip_list_iterator&
			operator=(const skip_list_iterator<IsConst>& rhs) noexcept
		{
			this->m_ptr = rhs.m_ptr;
			this->m_slist = rhs.m_slist;
		}

		constexpr skip_list_iterator& operator++()
		{
			this->m_ptr =
				(this->m_ptr ? this->m_ptr->m_next[0] : this->m_slist->m_header.m_next[0]);
			return *this;
		}

		// std::enable_if_t<Const, skip_list_iterator&>
		constexpr skip_list_iterator& operator--()
		{
			this->m_ptr =
				(this->m_ptr ? this->m_ptr->m_prev : this->m_slist->m_header.m_prev);
			return *this;
		}

		constexpr skip_list_iterator operator++(int)
		{
			auto old = *this;
			++* this;
			return old;
		}

		constexpr skip_list_iterator operator--(int)
		{
			auto old = *this;
			--* this;
			return old;
		}

		constexpr auto operator->() const noexcept
		{
			return &(this->operator*());
		}

		constexpr reference operator*() const noexcept
		{
			return this->m_ptr->m_data;
		}

		template <bool IsConst>
		constexpr bool operator==(const skip_list_iterator<IsConst>& rhs) const noexcept
		{
			return this->m_ptr == rhs.m_ptr;
		}

		template <bool IsConst>
		constexpr bool operator!=(const skip_list_iterator<IsConst>& rhs) const noexcept
		{
			return !this->operator==(rhs);
		}

	};


	template <typename Key, typename Value, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, Value>>>
	class map : public skip_list<std::pair<Key, Value>, Compare, Allocator, select1st<std::pair<const Key, Value>>>
	{
	public:
		auto& operator[](const Key& key)
		{
			auto iter = this->insert(std::make_pair(key, Value{ }));
			return iter.first->second;
		}
	};

	template <typename Key, typename Compare = std::less<Key>, typename KeyTraits = indentity<Key>>
	using pmr_skip_list = skip_list<Key, Compare, std::pmr::polymorphic_allocator<Key>, KeyTraits>;

	template <typename Key, typename Value, typename Compare = std::less<Key>>
	using pmr_map = map<Key, Value, Compare, std::pmr::polymorphic_allocator<std::pair<const Key, Value>>>;



} // namespace 

#include <ranges>

static_assert(std::ranges::bidirectional_range<leviathan::collections::skip_list<int>>);
static_assert(std::ranges::bidirectional_range<const leviathan::collections::skip_list<int>>);

#endif