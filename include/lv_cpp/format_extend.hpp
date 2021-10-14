#ifndef __FORMAT_EXTEND_HPP__
#define __FORMAT_EXTEND_HPP__

#include <iostream>
#include <tuple>
#include <vector>
#include <list>
#include <set>
#include <array>
#include <variant>
#include <format>

template <typename Container>
concept ContainerChecker = requires (const Container &c)
{
	// not support std::generator
	std::ranges::begin(c);
	std::ranges::end(c);
};
// tuple - like
template <typename Tuple>
concept TupleChecker = !ContainerChecker<Tuple> && requires (const Tuple & t)
{
	std::tuple_size<Tuple>::type;
};

template <ContainerChecker Ranges, typename CharT>
struct std::formatter<Ranges, CharT> 
	: std::formatter<std::ranges::range_value_t<Ranges>, CharT>
{
	template <typename FormatContext>
	auto format(const Ranges& rg, FormatContext& format_context)
	{
		using value_type = std::ranges::range_value_t<Ranges>;
		auto iter = std::formatter<char, CharT>().format('[', format_context);
		auto begin = std::ranges::begin(rg);
		auto end = std::ranges::end(rg);
		for (auto vec_iter = begin; vec_iter != end; ++vec_iter)
		{
			if (vec_iter != begin)
			{
				iter = ',', iter = ' ';
			}
			iter = std::formatter<value_type, CharT>().format(*vec_iter, format_context);
		}
		iter = ']';
		return iter;
	}
};

template <TupleChecker Tuple, typename CharT>
struct std::formatter<Tuple, CharT> : std::formatter<char, CharT>
{
	template <typename FormatContext>
	auto format(const Tuple& t, FormatContext& format_context)
	{
		auto __print_tuple = [&]<size_t... Idx>(std::index_sequence<Idx...>)
		{
			auto iter = std::formatter<char, CharT>().format('(', format_context);

			auto write = [&]<int Index>()
			{
				if constexpr (Index != 0)
					iter = ',', iter = ' ';
				using U = std::tuple_element_t<Index, Tuple>;
				iter = std::formatter<U, CharT>().format(std::get<Index>(t), format_context);
			};

			(write.operator() < Idx > (), ...);

			iter = ')';
			return iter;
		};
		constexpr auto size = std::tuple_size_v<Tuple>;
		return __print_tuple(std::make_index_sequence<size>());
	}
};
/*

void range_test()
{
	std::vector arr{ 1, 2, 3 };
	std::list ls{ 4, 5, 6 };
	std::set s{ 7, 8, 9 };
	int nested_arr[] = { 3, 2, 1 };
	std::string str = "hello world";
	std::cout << std::format
	("The vec = {}\nls = {}\nset = {}\nstr = {}\nchars = {}\nstatic_arr = {}\n"
		, arr, ls, s, str, 0, nested_arr);
}
void tuple_test()
{
	auto t = std::make_tuple(1, 2, 3.14);
	std::cout << std::format("tuple = {}\n", t);
}
int main()
{
	range_test();
	tuple_test();
	static_assert(TupleChecker<std::tuple<>>);
	return 0;
}


// template<typename _Tp, size_t Num>
// concept tuple_element_getable1 = requires(_Tp __t)
// { { std::get<Num>(__t) } -> std::convertible_to<const std::tuple_element_t<Num, _Tp>&>; };

// template<typename _Tp, size_t Num>
// concept tuple_element_getable2 = requires(_Tp __t)
// { { __t.template get<Num>() } -> std::convertible_to<const std::tuple_element_t<Num, _Tp>&>; };

template<typename _Tp, size_t Num>
concept has_tuple_element = requires(_Tp __t)
{
    typename std::tuple_size<_Tp>::type;
    requires (Num < std::tuple_size_v<_Tp>);
    typename std::tuple_element_t<Num, _Tp>;
    requires (
        // tuple_element_getable1<_Tp, Num> || 
        // tuple_element_getable2<_Tp, Num>
		(requires {{ std::get<Num>(__t) } -> std::convertible_to<const std::tuple_element_t<Num, _Tp>&>; }) || 
		(requires {{ __t.template get<Num>() } -> std::convertible_to<const std::tuple_element_t<Num, _Tp>&>; }) 
    );
};


template <typename Tuple, size_t Num>
struct check_one_element : std::bool_constant<has_tuple_element<Tuple, Num>> { };

template <typename Tuple, typename IndexSequence>
struct is_tuple_impl;

template <typename Tuple, size_t... Idx>
struct is_tuple_impl<Tuple, std::index_sequence<Idx...>> : std::conjunction<check_one_element<Tuple, Idx>...> { };

template <typename Tuple>
struct is_tuple : is_tuple_impl<Tuple, decltype(std::make_index_sequence<std::tuple_size_v<Tuple>>())>{ };

template <typename Tuple>
constexpr bool is_tuple_v = is_tuple<Tuple>::value;

static_assert(is_tuple_v<std::tuple<>>);
static_assert(is_tuple_v<std::tuple<int>>);
static_assert(is_tuple_v<std::pair<int, int>>);
static_assert(!is_tuple_v<int>);
*/


#endif