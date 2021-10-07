


#include <vector>
#include <algorithm>
#include <iostream>

template <typename T>
class sorted_list
{
	// trunk
	constexpr static int trunk_size = 1000;
	constexpr static auto last_element = []<typename U>(const U & vec)
	{ return vec.back(); };

public:

	sorted_list() = default;

	void insert(const T& val)
	{
		auto bucket = std::ranges::upper_bound(m_lists, val, {}, last_element);
		// if (m_lists.size())
        if (bucket != m_lists.end())
		{
			if (bucket == m_lists.end())
			{
				bucket--;
				bucket->emplace_back(val);
			}
			else 
			{
				auto iter = std::ranges::upper_bound(*bucket, val);
				bucket->insert(iter, val);
			}
			expand(std::ranges::distance(m_lists.begin(), bucket));
		}
		else
		{
			m_lists.emplace_back(std::vector<T>{ val });
			std::cout << "Empty List\n";
		}
		m_size++;
	}

	void insert_unique(const T& val)
	{
		auto exist = find(val) != nullptr;
		if (!exist)
			insert(val);
	}

	T* find(const T& val)
	{
		auto bucket_loc = std::ranges::lower_bound(m_lists, val, {}, last_element);
		if (bucket_loc == m_lists.end())
			return nullptr;

		auto iter = std::ranges::lower_bound(*bucket_loc, val);
		return *iter == val ? &(*iter) : nullptr;
	}

	bool remove(const T& val)
	{
		auto bucket_loc = std::ranges::lower_bound(m_lists, val, {}, last_element);
		if (bucket_loc == m_lists.end())
			return false;

		auto iter = std::ranges::lower_bound(*bucket_loc, val);
		if (*iter != val)
			return false;
		// remove op
		bucket_loc->erase(iter);
		m_size--;

	}

	void show() const
	{
		std::cout << "[";
		for (auto& vec : m_lists)
		{
			std::cout << "[";
			for (auto& val : vec)
				std::cout << val << ' ';
			std::cout << "]\n";
		}
		std::cout << "]\n";
	}

	size_t size() const
	{
		return m_size;
	}

private:
	std::vector<std::vector<T>> m_lists;
	size_t m_offset = 0;
	size_t m_size = 0;

	void expand(size_t pos)
	{
		if (m_lists[pos].size() > trunk_size)
		{
			auto& bucket = m_lists[pos];
			std::vector<T> half;
			// move
			half.reserve(std::ranges::distance(bucket.begin() + trunk_size, bucket.end()));
			half.insert(half.end(), bucket.begin() + trunk_size, bucket.end());
			bucket.erase(bucket.begin() + trunk_size, bucket.end());
			m_lists.insert(m_lists.begin() + pos + 1, std::move(half));
		}
	}

};




// #include "sorted_list.hpp"
#include <lv_cpp/utils/timer.hpp>
#include <iostream>
#include <random>
#include <set>
std::random_device rd;
std::vector<int> numbers;
std::set<int> ms;
sorted_list<int> ls;
void init()
{
	for (int i = 0; i < 1000 * 1000; ++i)
		numbers.push_back(i);
}

auto test_for_ms()
{
	std::cout << "STL Set\n";
	leviathan::timer _;
	for (auto val : numbers)
		ms.insert(val);
	return ms.size();
}

auto test_for_sorted_list()
{
	std::cout << "Sorted List\n";
	leviathan::timer _;
	for (auto val : numbers)
		ls.insert_unique(val);
	return ls.size();
}

int main()
{

	init();
	auto s1 = test_for_ms();
	auto s2 = test_for_sorted_list();
	std::cout << (s1 == s2) << '\n';
	auto ptr = ls.find(3);
	std::cout << (ptr == nullptr ? "NULL\n" : "Find it.\n");
	std::cout << "Ok\n";
}
