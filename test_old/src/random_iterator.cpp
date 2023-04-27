#include <lv_cpp/ranges/random_iterator.hpp>
#include <bits/stdc++.h>

template <typename D, typename G, typename... Args>
void test1(Args... args)
{
	leviathan::distribution_iterator<D, G> iter{ args... };
	for (int i = 0; i < 3; ++i)
	{
		std::cout << *iter << ' ';
		iter++;
	}
	static_assert(std::forward_iterator<decltype(iter)>);
	std::cout << std::is_nothrow_move_constructible_v<decltype(iter)> << '-';
	std::cout << std::is_nothrow_copy_constructible_v<decltype(iter)> << '-';
	std::cout << std::is_nothrow_move_assignable_v<decltype(iter)> << std::endl;
	std::endl(std::cout);
}



void test()
{
	std::random_device rd;
	test1<std::uniform_int_distribution<int>, std::mt19937_64>();
	test1<std::normal_distribution<double>, std::mt19937_64>(rd(), 0., 1.);
	test1<std::geometric_distribution<>, std::mt19937_64>();
	test1<std::gamma_distribution<>, std::mt19937_64>();
	test1<std::discrete_distribution<>, std::mt19937_64>();
}

int main()
{
    test();
}