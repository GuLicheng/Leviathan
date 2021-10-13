// #include "sorted_list.hpp"
// #include "Timer.h"
#include <lv_cpp/collections/sorted_list.hpp>
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
	for (int i = 0; i < 1000'000; ++i)
		numbers.push_back(rd());
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
		ls.insert(val);
	return ls.size();
}


void test_for_greater()
{
	sorted_list<int, std::greater<int>> ls0;
	for (int i = 0; i < 20; ++i)
	{
		ls0.insert(i);
	}
	ls0.show(1);
}

void test_for_erase()
{
	if (ms.size() != ls.size())
		throw std::runtime_error("Error size");
	for (auto val : numbers)
		ls.remove(val);
	std::cout << "After move, ls's size is ? " << ls.size() << '\n';
}

void test_for_search()
{
	for (auto val : ms)
	{
		auto ptr = ls.find(val);
		if (ptr == nullptr)
			throw std::runtime_error("Error for search");
	}
}

int main()
{

	init();
	auto s1 = test_for_ms();
	auto s2 = test_for_sorted_list();
	std::cout << (s1 == s2) << " " << s1 << '\n';
	auto ptr = ls.find(3);
	std::cout << (ptr == nullptr ? "NULL\n" : "Find it.\n");
	ls.show();
	//test_for_greater();
	test_for_search();
	test_for_erase();
	//ls.show(1);
	std::cout << "Ok\n";
}
