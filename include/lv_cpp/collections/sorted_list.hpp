#pragma once
#ifndef __SORTED_LIST_HPP__
#define __SORTED_LIST_HPP__

#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>

template <typename T, typename Compare = std::less<T>, bool Duplicate = false>
class sorted_list
{
	// trunk
	constexpr static size_t trunk_size = 1000;
	constexpr static auto last_element = []<typename U>(const U & vec)
	{
		return vec.back();
	};

public:

	sorted_list() = default;

	void insert(const T& val)
	{
		if (m_lists.size())
		{
			auto bucket_loc = std::ranges::lower_bound(m_lists, val, m_cmp, last_element);
			if (bucket_loc == m_lists.end())
			{
				m_lists.back().emplace_back(val);
				bucket_loc--;
			}
			else
			{
				auto iter = std::ranges::lower_bound(*bucket_loc, val, m_cmp);
				if constexpr (Duplicate)
				{
					if (*iter == val)
						return;
				}
				bucket_loc->insert(iter, val);
			}
			expand(std::ranges::distance(m_lists.begin(), bucket_loc));
		}
		else
		{
			m_lists.emplace_back(std::vector<T>{ val });
		}
		m_size++;
	}


	T* find(const T& val)
	{
		auto bucket_loc = std::ranges::lower_bound(m_lists, val, m_cmp, last_element);
		if (bucket_loc == m_lists.end())
			return nullptr;

		// the second lower_bound will always find a non-end position
		auto iter = std::ranges::lower_bound(*bucket_loc, val, m_cmp);
		return *iter == val ? &(*iter) : nullptr;
	}

	bool remove(const T& val)
	{
		auto bucket_loc = std::ranges::lower_bound(m_lists, val, m_cmp, last_element);
		if (bucket_loc == m_lists.end())
			return false;

		auto iter = std::ranges::lower_bound(*bucket_loc, val, m_cmp);
		if (*iter != val)
			return false;
		// remove op
		auto pos = std::ranges::distance(m_lists.begin(), bucket_loc);
		bucket_loc->erase(iter);
		m_size--;
		shrink(pos);
		return true;
	}

	void show(bool is_print = false) const
	{
		std::vector<T> ls;
		for (auto& vec : m_lists)
		{
			for (auto& val : vec)
				ls.push_back(val);
		}
		std::cout << "Is sorted ? " << (std::ranges::is_sorted(ls)) << '\n';
		if (is_print)
		{
			std::cout << '[';
			for (auto val : ls)
				std::cout << val << ' ';
			std::cout << "]\n";
		}
	}

	size_t size() const
	{
		return m_size;
	}

private:
	std::vector<std::vector<T>> m_lists;
	Compare m_cmp;
	size_t m_size = 0;

	void expand(size_t pos)
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
		}
	}

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



#endif