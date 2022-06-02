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

#include "config.hpp"

namespace leviathan::collections
{
/*
	Compare:
		bool binary(arg1, arg2): arg1: key of item, arg2: value


	A container which is faster than std::set/std::map

*/
template <typename T, typename Compare, bool Duplicate, typename Config, size_t TrunkSize = 1000>
class sorted_list_impl : public Config
{
	// trunk
	constexpr static size_t trunk_size = TrunkSize;

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

		constexpr reference operator*() const noexcept 
		{ return m_c->m_lists[m_out_idx][m_in_idx]; }

		constexpr auto operator->() const noexcept 
		{ return &(this->operator*()); }

		constexpr sorted_list_iterator& operator++() noexcept
		{
			m_in_idx++;
			if (m_in_idx == m_c->m_lists[m_out_idx].size())
				m_out_idx++, m_in_idx = 0;
			return *this;
		}

		constexpr sorted_list_iterator operator++(int) noexcept
		{
			auto old = *this;
			++ *this;
			return old;
		}

		constexpr sorted_list_iterator& operator--() noexcept
		{
			if (m_in_idx == 0) 
				m_in_idx = m_c->m_lists[--m_out_idx].size();
			m_in_idx --;
			return *this;
		}

		constexpr sorted_list_iterator operator--(int) noexcept
		{
			auto old = *this;
			-- *this;
			return old;
		}

	};

public:

	// In fact, it's unnecessary
	using iterator = sorted_list_iterator<false>; // -> decltype(rg.begin()) 
	using const_iterator = sorted_list_iterator<true>; // decltype(rg.as_const().begin())
	using reverse_iterator = std::reverse_iterator<iterator>; // std::reverse_iterator<1>
	using const_reverse_iterator = std::reverse_iterator<const_iterator>; // std::reverse_iterator<2>

	using typename Config::key_compare;
	using typename Config::value_compare;

	using typename Config::allocator_type;
	using typename Config::value_type;
	using typename Config::key_type;
	using typename Config::size_type;

	using Config::config;

	sorted_list_impl() noexcept(std::is_nothrow_default_constructible_v<key_compare>) = default;
	sorted_list_impl(Compare cmp) : m_cmp { cmp }, m_size { 0 } { }

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

	std::pair<iterator, bool> insert(const value_type& x) 
	{ return insert_impl(x); }

	std::pair<iterator, bool> insert(value_type&& x) 
	{ return insert_impl(std::move(x)); }

	// FIXME
	iterator insert(const_iterator, const value_type& x) 
	{ return insert_impl(x).first; }

	iterator insert(const_iterator, value_type&& x) 
	{ return insert_impl(std::move(x)).first; }
	
	template <typename... Args> 
	std::pair<iterator, bool> emplace(Args&&... x)
	{
		value_type e { x... };
		return insert_impl(std::move(e));
	}
	
	iterator erase(const_iterator pos) 
	{ return remove_impl(pos); }

	iterator erase(iterator pos) 
	{ return remove_impl(pos); }

	// iterator erase(const_iterator first, const_iterator last);

	template <typename K> size_type erase(const K& x) 
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

	void swap(sorted_list_impl& rhs) 
	noexcept(std::is_nothrow_swappable_v<outer_container> && std::is_nothrow_swappable_v<key_compare>)
	{
		m_lists.swap(rhs.m_lists);
		std::swap(m_size, rhs.m_size);
		std::swap(m_cmp, rhs.m_cmp);
	}

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


template <typename T, typename Compare = std::less<void>>
using sorted_list = sorted_list_impl<T, Compare, true, set_config<T, Compare, std::allocator<T>>>;

template <typename K, typename V, typename Compare = std::less<void>>
using sorted_map = sorted_list_impl<
					std::pair<K, V>,
					Compare, 
					true, 
					map_config<K, V, Compare, std::allocator<std::pair<const K, V>>>
				>;

}
#endif