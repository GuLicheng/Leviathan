#pragma once

#ifndef __SORTED_LIST_HPP__
#define __SORTED_LIST_HPP__


#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>
#include <compare>
#include <iterator>

#include <assert.h>

// for vector/list/deque/set/unordered_set
enum class config_type { set, map };

template <typename K, typename Allocator>
struct identity
{
    using key_type = K;
    using value_type = K;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    using allocator_type = Allocator;
    
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

	template <typename P1, typename P2>
	constexpr static bool compare_keys(const P1& p1, const P2& p2) noexcept
	{
		return p1 == p2;
	}

	template <typename U>
	constexpr static const auto& get_key(const U& x) noexcept
	{
		return x;
	}

};

// for map/unordered_map
template <typename K, typename V, typename Allocator>
struct select1st
{
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<const key_type, mapped_type>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    using allocator_type = Allocator;
    
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

	template <typename P1, typename P2>
	constexpr static bool compare_keys(const P1& p1, const P2& p2) noexcept
	{
		return p1.first == p2.first;
	}

	template <typename U>
	constexpr static const auto& get_key(const U& x) noexcept
	{
		return x.first;
	}

};

template <typename K, typename Compare, typename Allocator>
struct set_config : identity<K, Allocator>
{
    using key_compare = Compare;
	using typename identity<K, Allocator>::key_type;
	
	constexpr static auto config = config_type::set; 

	// we don't simply use Compare for value_compare since it may bring some extra cost.
	struct value_compare
	{

        constexpr value_compare(key_compare c) : comp{c} { }

		[[no_unique_address]] Compare comp;

		constexpr auto operator()(const key_type& lhs, const key_type& rhs) const
		noexcept(noexcept(comp(lhs, rhs)))
		{
			return comp(lhs, rhs);
		}

	};

};

template <typename K, typename V, typename Compare, typename Allocator>
struct map_config : select1st<K, V, Allocator>
{
    using key_compare = Compare;
	using typename select1st<K, V, Allocator>::value_type;
	using typename select1st<K, V, Allocator>::key_type;

	constexpr static auto config = config_type::map;

    struct value_compare
    {
        constexpr value_compare(key_compare c) : comp{c} { }

        constexpr auto operator()(const value_type& lhs, const value_type& rhs) const 
        noexcept(noexcept(comp(lhs.first, rhs.first)))
        {
            return comp(lhs.first, rhs.first);
        }

        [[no_unique_address]] key_compare comp;
    };

};

/*
	Compare:
		bool binary(arg1, arg2): arg1: K, arg2: key_type
*/
template <typename T, typename Compare, bool Duplicate, typename Config>
class sorted_list_impl : public Config
{
	// trunk
	constexpr static size_t trunk_size = 1000;

	static_assert(Duplicate, "MultiSet and MultiMap is not support now");

	using inner_container = std::vector<T>;
	using outer_container = std::vector<inner_container>;
	using self_type = sorted_list_impl;

	template <bool Const>
	struct sorted_list_iterator
	{

		using in_iter = typename inner_container::iterator;
		using out_iter = typename outer_container::iterator;

		using link_type = std::conditional_t<Const, const sorted_list_impl*, sorted_list_impl*>;

		using value_type = std::conditional_t<Const,
			const typename std::iterator_traits<in_iter>::value_type,
			typename std::iterator_traits<in_iter>::value_type>;

		using reference = value_type&;

		using difference_type = typename std::iterator_traits<in_iter>::difference_type;
		using iterator_category = std::bidirectional_iterator_tag;


		link_type m_c;
		std::size_t m_out_idx;
		std::size_t m_in_idx;

		constexpr sorted_list_iterator() noexcept = default;
		constexpr sorted_list_iterator(const sorted_list_iterator&) noexcept = default;

		constexpr sorted_list_iterator(const sorted_list_iterator<!Const>& rhs) noexcept requires (Const)
			: m_c{ rhs.m_c }, m_out_idx{ rhs.m_out_idx }, m_in_idx{ rhs.m_in_idx } { }

		constexpr sorted_list_iterator(link_type c, std::size_t out_idx, std::size_t in_idx = 0)
			: m_c{ c }, m_out_idx{ out_idx }, m_in_idx{ in_idx } { }

		constexpr bool operator==(const sorted_list_iterator& rhs) const noexcept = default;

		constexpr reference operator*() const noexcept { return m_c->m_lists[m_out_idx][m_in_idx]; }
		constexpr auto operator->() const noexcept { return &(this->operator*()); }

		sorted_list_iterator& operator++() 
		{
			m_in_idx++;
			if (m_in_idx == m_c->m_lists[m_out_idx].size())
				m_out_idx++, m_in_idx = 0;
			return *this;
		}

		sorted_list_iterator operator++(int)
		{
			auto old = *this;
			++ *this;
			return old;
		}

		sorted_list_iterator& operator--() 
		{
			if (m_in_idx == 0) 
				m_in_idx = m_c->m_lists[--m_out_idx].size();
			m_in_idx --;
			return *this;
		}

		sorted_list_iterator operator--(int)
		{
			auto old = *this;
			-- *this;
			return old;
		}

	};

public:

	using iterator = sorted_list_iterator<false>;
	using const_iterator = sorted_list_iterator<true>;
	using reversed_iterator = std::reverse_iterator<iterator>;
	using const_reversed_iterator = std::reverse_iterator<const_iterator>;

	// using key_compare = typename Config::key_compare;
	// using value_compare = typename Config::value_compare;

	using typename Config::key_compare;
	using typename Config::value_compare;

	using typename Config::allocator_type;
	using typename Config::value_type;
	using typename Config::key_type;
	using typename Config::size_type;

	using Config::config;

	sorted_list_impl() = default;


	size_type size() const noexcept 
	{ return m_size; }
	
	bool empty() const noexcept 
	{ return m_size == 0; }

	// simple API for iterator
	iterator begin() noexcept 
	{ return { this, 0 }; }
	
	iterator end() noexcept 
	{ return { this, m_lists.size() }; }

	const_iterator begin() const noexcept 
	{ return const_cast<self_type&>(*this).begin(); }

	const_iterator end() const noexcept 
	{ return const_cast<self_type&>(*this).end(); }

	const_iterator cbegin() const noexcept 
	{ return begin(); }

	const_iterator cend() const noexcept 
	{ return end(); }

	reversed_iterator rbegin() noexcept 
	{ return std::make_reverse_iterator(end()); } 

	reversed_iterator rend() noexcept 
	{ return std::make_reverse_iterator(begin()); } 

	const_reversed_iterator rbegin() const noexcept 
	{ return std::make_reverse_iterator(end()); } 

	const_reversed_iterator rend() const noexcept 
	{ return std::make_reverse_iterator(begin()); } 
	
	const_reversed_iterator rcbegin() const noexcept 
	{ return rbegin(); } 

	const_reversed_iterator rcend() const noexcept 
	{ return rend(); } 

	std::pair<iterator, bool> insert(const value_type& x) 
	{ return insert_impl(x); }

	std::pair<iterator, bool> insert(value_type&& x) 
	{ return insert_impl(std::move(x)); }

	// FIXME
	iterator insert(const_iterator, const value_type& x) 
	{ return insert_impl(x).first; }

	iterator insert(const_iterator, value_type&& x) 
	{ return insert_impl(std::move(x)).first; }
	
	// template <typename... Args> 
	// std::pair<iterator, bool> emplace(Args&&... x);
	
	iterator erase(const_iterator pos) 
	{ return remove_impl(pos); }
	// iterator erase(const_iterator first, const_iterator last);

	size_t erase(const key_type& x) 
	{ return remove_impl(x); }
	// template <typename K> size_type erase(K&& x);

	// iterator find(const key_type& x) noexcept { return find_impl(x); }
	// const_iterator find(const key_type& x) const noexcept { return const_cast<self_type&>(*this).find_impl(x); }
	template <typename K> iterator find(const K& x) noexcept 
	{ return find_impl(x); }

	template <typename K> const_iterator find(const K& x) const noexcept 
	{ return const_cast<self_type&>(*this).find_impl(x); }

	// TODO:
	template <typename K> iterator lower_bound(const K& x) noexcept 
	{ return lower_bound_impl(x); }

	template <typename K> const_iterator lower_bound(const K& x) const noexcept 
	{ return const_cast<self_type&>(*this).lower_bound_impl(x); }

	template <typename K> iterator upper_bound(const K& x) noexcept 
	{ return upper_bound_impl(x); }

	template <typename K> const_iterator upper_bound(const K& x) const noexcept 
	{ return const_cast<self_type&>(*this).upper_bound_impl(x); }

	template <typename K> 
	std::pair<iterator, iterator> equal_range(const K& x) noexcept
	{ return equal_range_impl(x); }

	template <typename K> 
	std::pair<const_iterator, const_iterator> equal_range(const K& x) const noexcept
	{ return const_cast<self_type&>(*this).equal_range_impl(x); }

	// bool contains(const key_type& x) const noexcept { return find(x) != end(); } 
	template <typename K> bool contains(const K& x) const noexcept 
	{ return find(x) != end(); } 

	void clear()
	{ 
		m_lists.clear(); 
		m_size = 0; 
	}

	template <typename K> 
	auto& operator[](K&& k) requires (config == config_type::map)
	{ return insert(std::make_pair((K&&) k, typename Config::mapped_type())).first->second; }	


private:
	outer_container m_lists;
	[[no_unique_address]] key_compare m_cmp = { key_compare() };
	size_type m_size = 0;


	// return lower_bound item
	template <typename K>
	std::pair<std::size_t, std::size_t> find_item_by_key(const K& k) const 
	{
		auto bucket_loc = std::lower_bound(m_lists.begin(), m_lists.end(), k, [this](const auto& vec, const auto& value) { 
			return m_cmp(Config::get_key(vec.back()), value); 
		});
		
		if (bucket_loc == m_lists.end())
			return { m_lists.size(), 0 };
		auto iter = std::lower_bound(bucket_loc->begin(), bucket_loc->end(), k, [this](const auto& item, const auto& value) { 
			return m_cmp(Config::get_key(item), value);
		});
		return { std::ranges::distance(m_lists.begin(), bucket_loc), std::ranges::distance(bucket_loc->begin(), iter) };
	}

	template <typename U> 
	std::pair<iterator, bool> insert_impl(U&& val) requires (Duplicate)
	{
		bool succeed;
		std::size_t out_idx, in_idx;
		if (m_lists.size())
		{
			auto [i, j] = find_item_by_key(Config::get_key(val));
			// std::cout << i << '-' << j << '\n';
			if (i == m_lists.size())
			{
				// insert last position
				m_lists.back().emplace_back((U&&) val);
				i = m_lists.size() - 1;
				j = m_lists.back().size() - 1;
				succeed = true;
			}
			else
			{
				if (Config::get_key(m_lists[i][j]) == Config::get_key(val))
				{
					succeed = false;
				}
				else
				{
					auto& v = m_lists[i];
					auto ret_iter = v.insert(v.begin() + j, (U&&)val);
					j = std::ranges::distance(v.begin(), ret_iter);
					succeed = true;
				}
			}

			bool expanded = expand(i);
			if (expanded && j > trunk_size)
			{
				i++;
				j -= trunk_size;
			}
			out_idx = i, in_idx = j;
		}
		else
		{
			m_lists.emplace_back(inner_container{ (U&&) val });
			out_idx = in_idx = 0;
			succeed = true;
		}
		if (succeed) m_size++;
		return { iterator(this, out_idx, in_idx), succeed };
	}

	template <typename U> 
	std::pair<iterator, bool> insert_impl(U&& val)
	{
		struct NotImplErr { };
		throw NotImplErr{ };
	}

	template <typename K>
	iterator find_impl(const K& val) noexcept
	{
		auto [i, j] = find_item_by_key(val);

		if (i == m_lists.size())
			return end();

		return Config::get_key(m_lists[i][j]) == val ? 
				iterator(this, i, j) :
				end();
	}

	template <typename K>
	iterator lower_bound_impl(const K& val) noexcept
	{
		auto [i, j] = find_item_by_key(val);
		return i == m_lists.size() ? end() : iterator(this, i, j);
	}

	template <typename K>
	iterator upper_bound_impl(const K& val) noexcept
	{
		auto lower = lower_bound_impl(val);
		return std::find_if(lower, end(), [&](const auto& x){
			return Config::get_key(x) != val;
		});
	}

	template <typename K>
	std::pair<iterator, iterator> equal_range_impl(const K& val) noexcept
	{
		auto lower = lower_bound_impl(val);
		auto upper = std::find_if(lower, end(), [&](const auto& x){
			return Config::get_key(x) != val;
		});
		return { lower, upper };
	}

	// FIXME: while (iter != end) iter = erase(iter);
	iterator remove_impl(const_iterator pos) requires (Duplicate)
	{
		auto i = pos.m_out_idx, j = pos.m_in_idx;
		m_lists[i].erase(m_lists[i].begin() + j);
		--m_size;

		// shrink(i);	
		if (m_lists[i].empty())
		{
			m_lists.erase(m_lists.begin() + i);
			return { this, i, j };
		}

		if (j == m_lists[i].size())
			return { this, i + 1, 0 };

		return { this, i, j };
	}

	size_type remove_impl(const key_type& x) requires (Duplicate)
	{
		auto pos = find(x);
		if (pos == end())
			return 0;
		erase(pos);
		return 1;
	}

	// move elements more than truck_size in bucket[pos] into next bucket
	bool expand(size_t pos)
	{
		if (m_lists[pos].size() > trunk_size * 2)
		{
			auto& bucket = m_lists[pos];
			std::vector<T> half;
			// move
			half.reserve(std::ranges::distance(bucket.begin() + trunk_size, bucket.end()));
			if constexpr (std::is_nothrow_constructible_v<T>)
			{
				half.insert(half.end(),
					std::make_move_iterator(bucket.begin() + trunk_size),
					std::make_move_iterator(bucket.end()));
			}
			else
			{
				half.insert(half.end(), bucket.begin() + trunk_size, bucket.end());
			}
			bucket.erase(bucket.begin() + trunk_size, bucket.end());
			m_lists.insert(m_lists.begin() + pos + 1, std::move(half));
			return true;
		}
		return false;
	}

	// Make sure that each bucket will not be empty !
	void shrink(size_t pos)
	{
		if (m_lists[pos].empty())
			m_lists.erase(m_lists.begin() + pos);
		return;
		//if (m_lists[pos].size() < trunk_size / 2 && m_lists.size() > 1)
		//{
		//	if (pos == 0)
		//		pos++;
		//	auto prev = pos - 1;
		//	m_lists[prev].insert(m_lists[prev].end(), m_lists[pos].begin(), m_lists[pos].end());
		//	if (m_lists[prev].size() < trunk_size * 2)
		//		m_lists.erase(m_lists.begin() + pos);
		//	else
		//	{
		//		m_lists[pos].clear();
		//		m_lists[pos].insert(m_lists[pos].end(), m_lists[prev].begin() + m_lists.size() / 2, m_lists[prev].end());
		//		m_lists[prev].erase(m_lists[prev].begin() + m_lists.size() / 2, m_lists[prev].end());
		//	}
		//}
	}

};


template <typename T, typename Compare = std::less<T>>
using sorted_list = sorted_list_impl<T, Compare, true, set_config<T, Compare, std::allocator<T>>>;

template <typename K, typename V, typename Compare = std::less<K>>
using sorted_map = sorted_list_impl<
					std::pair<K, V>,
					Compare, 
					true, 
					map_config<K, V, Compare, std::allocator<std::pair<K, V>>>
				>;


#endif