#include <iostream>
#include <vector>
#include <lv_cpp/tuples/algorithm2.hpp>
#include <lv_cpp/tuples/hash.hpp>
#include <lv_cpp/format_extend.hpp>

int main()
{
	using namespace leviathan::tuple;
	auto t = std::make_tuple(1, 3.14, "Hello world");
	tuple_dynamic_set(t, [](auto& x) { x = 0; }, 1);
	tuple_print(t, std::cout);
	auto printer = []<typename T>(const T & val) { std::cout << val << ' '; }; 

	auto res = tuple_reduce(std::make_tuple(1, 2, 3), std::plus<>(), 0);
	std::cout << res << '\n';
	std::cout << tuple_inner_preduct(std::make_tuple(1, 2, 3), std::make_tuple(1, 2, 3), std::plus<>(), std::plus<>(), 0);

	std::cout << std::format("Tuple = {}\n", t);
}