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


#endif