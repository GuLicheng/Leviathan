#pragma once

#include <lv_cpp/collections/config.hpp>

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <array>
#include <iterator>
#include <memory_resource>
#include <type_traits>

namespace leviathan::collections
{

	/*
		Template Params:
			MaxLevel: max level of each node
			Ratio: reciprocal of possibility 
	*/
	template <typename T, typename Compare, typename Allocator , typename Config, bool Duplicate = true, int MaxLevel = 32, int Ratio = 4>
	class skip_list_impl : public Config
	{
		static_assert(Duplicate, "Not support multi-key now");

	public:

		constexpr static int MAXLEVEL = MaxLevel;
		
		inline static std::random_device rd;

		constexpr static std::random_device::result_type p = std::random_device::max() / Ratio;

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
			explicit header(Derived* prev, Derived* next, std::size_t num = MAXLEVEL) noexcept
				: m_prev{ prev }, m_next{ num, next } { } // make it noexcept

			// use self instead of nullptr as sentinel
			header() noexcept
				: m_prev{ derived_ptr() }, m_next{ MAXLEVEL, derived_ptr() } { }

			auto derived_ptr() noexcept
			{ return static_cast<Derived*>(this); }

			auto derived_ptr() const noexcept
			{ return static_cast<const Derived*>(this); }

			void reset() noexcept
			{
				m_prev = derived_ptr();
				std::ranges::fill(m_next, derived_ptr());
			}
		};

		struct skip_node : public header<skip_node>
		{
			typename Config::value_type m_data;
			using base = header<skip_node>;

			skip_node(const T& x, skip_node* next, std::size_t next_num)
				: base(this, next, next_num), m_data(x) { }
			skip_node(T&& x, skip_node* next, std::size_t next_num)
				: base(this, next, next_num), m_data(std::move(x)) { }
		};

		using key_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<skip_node>;
		using self_type = skip_list_impl;
		using allocator_traits = std::allocator_traits<key_allocator_type>;

		template <bool Const>
		struct skip_list_iterator
		{

			using link_type = std::conditional_t<Const, const skip_node*, skip_node*>;

			using value_type = std::conditional_t<Const, 
				const typename Config::value_type, 
				typename Config::value_type>;

			using reference = value_type&;

			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;


			link_type m_ptr;

			constexpr skip_list_iterator() noexcept = default;
			constexpr skip_list_iterator(const skip_list_iterator&) noexcept = default;

			constexpr skip_list_iterator(const skip_list_iterator<!Const>& rhs) noexcept requires (Const)
				: m_ptr{ rhs.m_ptr } { }

			constexpr skip_list_iterator(link_type ptr) noexcept
				: m_ptr{ ptr } { }

			constexpr skip_list_iterator& operator++() noexcept
			{
				m_ptr = m_ptr->m_next[0];
				return *this;
			}

			constexpr skip_list_iterator& operator--() noexcept
			{
				m_ptr = m_ptr->m_prev;
				return *this;
			}

			constexpr skip_list_iterator operator++(int) noexcept
			{
				auto old = *this;
				++* this;
				return old;
			}

			constexpr skip_list_iterator operator--(int) noexcept
			{
				auto old = *this;
				--* this;
				return old;
			}

			constexpr auto operator->() const noexcept
			{ return &(m_ptr->m_data); }

			constexpr reference operator*() const noexcept
			{ return m_ptr->m_data; }

			// (skip_list_iterator rhs) may be better ?
			constexpr bool operator==(const skip_list_iterator& rhs) const noexcept = default;

		};

		
	public:

		using const_iterator = skip_list_iterator<true>;
		using iterator = skip_list_iterator<false>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = reverse_iterator;

		using typename Config::key_compare;
		using typename Config::value_compare;

		using typename Config::allocator_type;
		using typename Config::value_type;
		using typename Config::key_type;
		using typename Config::size_type;

		using Config::config;

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
		skip_list_impl() noexcept(noexcept(std::is_nothrow_default_constructible_v<key_allocator_type>
			&& std::is_nothrow_default_constructible_v<Compare>))
			: m_cmp{ }, m_alloc{ }, m_size{ 0 }, m_header{ }, m_level{ 1 }
		{
		}

		skip_list_impl(const allocator_type& alloc)
			: m_cmp{ }, m_alloc{ alloc }, m_size{ 0 }, m_header{ }, m_level{ 1 } 
		{
		}

		~skip_list_impl() noexcept
		{
			clear();
		}
		// https://github.com/CppCon/CppCon2017 - How to Write a Custom Allocator 
		// https://github.com/CppCon/CppCon2017/blob/master/Tutorials/How%20to%20Write%20a%20Custom%20Allocator/How%20to%20Write%20a%20Custom%20Allocator%20-%20Bob%20Steagall%20-%20CppCon%202017.pdf
		skip_list_impl(const skip_list_impl& rhs)
			: m_cmp{ rhs.m_cmp }, m_size{ }, m_header{ }, m_level{ 1 }
		{
			// may use impl to warp all flied but alloc
			// traits::select_on_container_copy_construction(rhs.m_alloc)
			m_alloc = std::allocator_traits<key_allocator_type>::select_on_container_copy_construction(rhs.m_alloc);
			// assign_from(rhs.cbegin(), rhs.cend());
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

		skip_list_impl(skip_list_impl&& rhs) 
		noexcept(isNoexceptMoveConstruct)
			: m_cmp{ std::move(rhs.m_cmp) }, m_alloc{ std::move(rhs.m_alloc) }, m_size{ rhs.m_size }, m_header{ std::move(rhs.m_header) }, m_level{ rhs.m_level }
		{
		}

		skip_list_impl& operator=(const skip_list_impl& rhs)
		{
			if (std::addressof(rhs) != this)
			{
				// std::true_type
				if constexpr (typename std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment())
				{
					if (m_alloc != rhs.m_alloc)
					{
						// clear_and_deallocate_memory()
						clear_and_deallocate_memory();
					}
					m_alloc = rhs.m_alloc;
				}
				// assign_from(rhs.begin(), rhs.end());
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
		skip_list_impl& operator=(skip_list_impl&& rhs) 
		noexcept(isNoexceptMoveAssign)
		{
			if (this != std::addressof(rhs))
			{
				if constexpr (typename std::allocator_traits<key_allocator_type>::propagate_on_container_move_assignment())
				{
					// clear_and_deallocate_memory
					// move alloc and impl
					clear_and_deallocate_memory();
					m_alloc = std::move(rhs.m_alloc);
					m_header = std::move(rhs.m_header);
					m_size = rhs.m_size;
					m_cmp = std::move(rhs.m_cmp);
					m_level = rhs.m_level;
				}
				else if (typename std::allocator_traits<key_allocator_type>::is_always_equal() || m_alloc == rhs.m_alloc)
				{
					// clear_and_deallocate_memory()
					// impl = move(rhs.impl)
					clear_and_deallocate_memory();
					m_header = std::move(rhs.m_header);
					m_size = rhs.m_size;
					m_cmp = std::move(rhs.m_cmp);
					m_level = rhs.m_level;
				}
				else
				{
					// assign(move_iter(rhs.begin()), move_iter(rhs.end()));
					assign_from(std::make_move_iterator(rhs.begin()), std::make_move_iterator(rhs.end()));
				}
			}
			return *this;
		}

		void swap(skip_list_impl& rhs)
		{
			if (this != std::addressof(rhs))
			{
				if constexpr (typename std::allocator_traits<key_allocator_type>::propagate_on_container_swap())
				{
					// std::swap(impl and alloc)
					swap_impl(rhs);
					std::swap(m_alloc, rhs.m_alloc);
				}
				else if (typename std::allocator_traits<key_allocator_type>::is_always_equal()
					|| m_alloc == rhs.m_alloc)
				{
					swap_impl(rhs);
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
			m_size = 0;
			m_header.reset();
		}

		// simple API for iterator
		iterator begin() noexcept 
		{ return { m_header.m_next[0] }; }
		
		iterator end() noexcept 
		{ return { m_header.derived_ptr() }; }

		const_iterator begin() const noexcept 
		{ return const_cast<self_type&>(*this).begin(); }

		const_iterator end() const noexcept 
		{ return const_cast<self_type&>(*this).end(); }

		const_iterator cbegin() const noexcept 
		{ return begin(); }

		const_iterator cend() const noexcept 
		{ return end(); }

		reverse_iterator rbegin() noexcept 
		{ return std::make_reverse_iterator(end()); } 

		reverse_iterator rend() noexcept 
		{ return std::make_reverse_iterator(begin()); } 

		const_reverse_iterator rbegin() const noexcept 
		{ return std::make_reverse_iterator(end()); } 

		const_reverse_iterator rend() const noexcept 
		{ return std::make_reverse_iterator(begin()); } 
		
		const_reverse_iterator rcbegin() const noexcept 
		{ return rbegin(); } 

		const_reverse_iterator rcend() const noexcept 
		{ return rend(); } 

		std::size_t size() const noexcept
		{ return m_size; }

		bool empty() const noexcept
		{ return size() == 0; }

		template <typename K> iterator find(const K& x) noexcept 
		{
			auto lower = lower_bound(x);
			if (lower != end() && Config::get_key(*lower) == x)
				return lower;
			return end(); 
		}

		template <typename K> const_iterator find(const K& x) const noexcept 
		{ return const_cast<self_type&>(*this).find(x); }

		template <typename K> iterator lower_bound(const K& x) noexcept 
		{ return lower_bound_impl(x); }

		template <typename K> const_iterator lower_bound(const K& x) const noexcept 
		{ return const_cast<self_type&>(*this).lower_bound_impl(x); }

		template <typename K> iterator upper_bound(const K& x) noexcept 
		{
			auto lower = lower_bound(x);
			return std::find_if(lower, end(), [&](const auto& item){
				return Config::get_key(item) != x;
			});
		}

		template <typename K> const_iterator upper_bound(const K& x) const noexcept 
		{ return const_cast<self_type&>(*this).upper_bound(x); }

		template <typename K>
		std::pair<iterator, iterator> equal_range(const K& x) noexcept
		{
			auto lower = lower_bound(x);
			auto upper = std::find_if(lower, end(), [&](const auto& item){
				return Config::get_key(item) != x;
			});
			return { lower, upper };
		}

		template <typename K> 
		std::pair<const_iterator, const_iterator> equal_range(const K& x) const noexcept
		{ return const_cast<self_type&>(*this).equal_range(x); }

		template <typename K> bool contains(const K& x) const noexcept 
		{ return find(x) != end(); } 

		std::pair<iterator, bool> insert(const value_type& x) 
		{ return insert_unique(x); }

		std::pair<iterator, bool> insert(value_type&& x) 
		{ return insert_unique(std::move(x)); }

		template <typename K>
		iterator insert(const_iterator, K&& x) 
		{ return insert_unique((K&&)x).first; }

		template <typename K>
		iterator insert(iterator, K&& x) 
		{ return insert_unique((K&&)x).first; }

		template <typename K> 
		auto& operator[](K&& k) requires (config == config_type::map)
		{ return insert(std::make_pair((K&&) k, typename Config::mapped_type())).first->second; }	

		iterator erase(const_iterator pos) 
		{ return erase_node(Config::get_key(*pos)); }

		iterator erase(iterator pos) 
		{ return erase_node(Config::get_key(*pos)); }

		template <typename K> size_type erase(const K& x) 
		{
			auto old_size = size();
			erase_node(x);
			return old_size - size();
		}

		template <typename... Args>
		std::pair<iterator, bool> emplace(Args&&... args)
		{ return emplace_unique((Args&&) args...); }

	private:
		[[no_unique_address]] Compare m_cmp;
		[[no_unique_address]] key_allocator_type m_alloc;
		std::size_t m_size;
		header<skip_node> m_header;
		int m_level;

		auto sentinel_node() noexcept
		{ return m_header.derived_ptr(); }

		// auto sentinel_node() const noexcept
		// { return m_header.derived_ptr(); }

		bool is_sentinel(const skip_node* node) const noexcept
		{ return node == m_header.derived_ptr(); }

		template <typename Iter, typename Sent>
		void assign_from(Iter first, Sent last)
		{
			for (auto iter = first; iter != last; ++iter)
				insert(*iter);
		}

		void clear_and_deallocate_memory()
		{
			// if (m_header.m_next.empty())
			// 	return;
			assert(m_header.m_next.size() > 0);
			auto cur = m_header.m_next[0];
			while (!is_sentinel(cur))
			{
				auto next = cur->m_next[0];
				destroy_one_node(cur);
				cur = next;
			}
		}

		// return target node if succeed else prev position of target node for inserting
		template <bool CompletelyPrev, typename K>
		std::pair<skip_node*, bool> find_node_with_prev(const K& val, std::array<skip_node*, MAXLEVEL>& prev) noexcept
		{
			skip_node* cur = m_header.derived_ptr();
			bool exist = false;
			for (int i = m_level - 1; i >= 0; --i)
			{
				for (; !is_sentinel(cur->m_next[i]) && m_cmp(Config::get_key(cur->m_next[i]->m_data), val); cur = cur->m_next[i]);
				auto next_node = cur->m_next[i];
				if (!is_sentinel(next_node) && Config::get_key(next_node->m_data) == val)
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
			if (!is_sentinel(pos->m_next[0]))
				pos->m_next[0]->m_prev = new_node;
			// update next
			const int level = static_cast<int>(new_node->m_next.size());
			for (int i = 0; i < level; ++i)
			{
				if (i >= m_level)
					m_header.m_next[i] = new_node;
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
			if (is_sentinel(pos->m_prev))
				m_header.m_next[0] = pos;
			// check tail
			if (is_sentinel(pos->m_next[0]))
				m_header.m_prev = pos;
		}

		// assume std::is_nothrow_swappable_v<Compare>
		void swap_impl(skip_list_impl& rhs) noexcept
		{
			std::swap(m_size, rhs.m_size);
			std::swap(m_header, rhs.m_header);
			std::swap(m_cmp, rhs.m_cmp);
			std::swap(m_level, rhs.m_level);	
		}

		// find node without prev
		template <typename K>
		std::pair<skip_node*, bool> find_node(const K& val) 
		{
			skip_node* cur = m_header.derived_ptr();
			for (int i = m_level; i >= 0; --i)
			{
				for (; !is_sentinel(cur->m_next[i]) && m_cmp(Config::get_key(cur->m_next[i]->m_data), val); cur = cur->m_next[i]);
				auto next_node = cur->m_next[i];
				// if (next_node && KeyTraits::compare(std::equal_to<>(), next_node->m_data, val))
				if (!is_sentinel(next_node) && Config::get_key(next_node->m_data) == val)
				{
					return { cur->m_next[i], true };
				}
			}
			return { cur, false };
		}

		template <typename U>
		std::pair<skip_node*, bool> insert_unique(U&& val)
		{
			std::array<skip_node*, MAXLEVEL> prev;
			prev.fill(sentinel_node());
			auto [cur, exist] = find_node_with_prev<false>(Config::get_key(val), prev);
			if (exist)
				return { cur, false };

			// insert a new node
			auto level = get_level();
			auto new_node = create_one_node(level, (U&&) val, sentinel_node());
			insert_and_update_all_member(cur, new_node, prev, level);
			return { new_node, true };
		}

		// insert and emplace will always recept a value_type
		template <typename... Args>
		std::pair<skip_node*, bool> emplace_unique(Args&&... args)
		{
			auto level = get_level();
			auto new_node = create_one_node(level, (Args&&)args..., sentinel_node());
			std::array<skip_node*, MAXLEVEL> prev;
			prev.fill(sentinel_node());
			auto [cur, exist] = find_node_with_prev<false>(Config::get_key(new_node->m_data), prev);
			if (exist)
			{
				destroy_one_node(new_node);
				return { cur, false };
			}
			insert_and_update_all_member(cur, new_node, prev, level);
			return { new_node, true };
		}

		void insert_and_update_all_member(skip_node* pos, skip_node* new_node, std::array<skip_node*, MAXLEVEL>& prev, int new_level)
		{
			insert_after(pos, new_node, prev);
			update_header(new_node);
			m_level = std::max(m_level, new_level);
			++m_size;
		}

		skip_node* erase_node(const key_type& val)
		{
			std::array<skip_node*, MAXLEVEL> prev;
			prev.fill(sentinel_node());
			auto [cur, exist] = find_node_with_prev<true>(val, prev);
			if (!exist)
				return cur->m_next[0];

			skip_node* deleted_node = cur->m_next[0];
			if (deleted_node->m_next[0])
				deleted_node->m_next[0]->m_prev = cur;
			for (std::size_t i = 0; i < deleted_node->m_next.size(); ++i)
				prev[i]->m_next[i] = deleted_node->m_next[i];

			int new_level;
			for (new_level = m_level - 1; new_level >= 0 && is_sentinel(m_header.m_next[new_level]); --new_level);
			m_level = new_level + 1;

			// int new_level = m_level;
			// while (--new_level >= 0 && m_header.m_next[new_level] == nullptr)
			// 	m_level = new_level;
			// std::destroy
			destroy_one_node(deleted_node);
			--m_size;
			return prev[0]->m_next[0];
		}

		template <typename K>
		skip_node* lower_bound_impl(const K& val) 
		{
			auto [cur, exist] = find_node(val);
			return exist ? cur : cur->m_next[0];
		}

		void destroy_one_node(skip_node* p) noexcept // assume destructor and deallocate is exception-safe
		{
			allocator_traits::destroy(m_alloc, p);
			m_alloc.deallocate(p, sizeof(skip_node));
		}

		template <typename... Args>
		skip_node* create_one_node(int level, Args&&... args)
		{
			auto addr = m_alloc.allocate(1);
			assert(addr != nullptr); // if (!addr) throw std::bad_alloc { }
			allocator_traits::construct(m_alloc, addr, (Args&&)args..., level);
			return addr;
		}

	};


	// template <
	// typename T, 
	// typename Compare, 
	// typename Allocator, 
	// typename Config, 
	// bool Duplicate = true, 
	// int MaxLevel = 32, 
	// int Ratio = 4>
	// class skip_list_impl

	template <typename T, typename Compare = std::less<void>, typename Allocator = std::allocator<T>>
	using skip_list = skip_list_impl<T, Compare, Allocator, set_config<T, Compare, Allocator>, true>;

	template <typename K, typename V, typename Compare = std::less<void>, typename Allocator = std::allocator<std::pair<K, V>>>
	using skip_map = skip_list_impl<std::pair<K, V>, Compare, Allocator, map_config<K, V, Compare, Allocator>, true>;


} // namespace 


