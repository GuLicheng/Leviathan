#ifndef __TUPLE_ALGORITHM_HPP__
#define __TUPLE_ALGORITHM_HPP__

#include "tuple_base.hpp"
#include "static_looer.hpp"
#include <functional>

namespace leviathan::tuple
{

	template <typename T>
	concept tuple_or_reference = tuple_like<remove_cvref_t<T>>;

	template <tuple_or_reference Tuple, typename UnaryFunction, typename Proj = std::identity>
	constexpr void tuple_for_each(Tuple&& t, UnaryFunction func, Proj proj = {})
	{
		constexpr auto size = tuple_size_v<remove_cvref_t<Tuple>>;
		auto invoker = [&]<size_t... Idx>(std::index_sequence<Idx...>)
		{
			(std::invoke(func, std::invoke(proj, get<Idx>(t))), ...);
		};
		invoker(std::make_index_sequence<size>());
	}

	template <tuple_or_reference Tuple, typename OStream>
	constexpr void tuple_print(Tuple&& t, OStream& os)
	{
		os << '(';
		[&]<size_t... Idx>(std::index_sequence<Idx...>)
		{
			((Idx == 0 ? os << get<Idx>(t) : os << ", " << get<Idx>(t)), ...);
		}(std::make_index_sequence<tuple_size_v<remove_cvref_t<Tuple>>>());
		os << ')';
	}

	template <tuple_or_reference Tuple, typename BinaryFunc, typename Init>
	constexpr auto tuple_reduce(const Tuple& t, BinaryFunc func, Init init)
	{
		constexpr auto size = tuple_size_v<Tuple>;
		return static_looper<0, size>::reduce(t, std::move(func), std::move(init));
	}

	template <tuple_or_reference Tuple1, tuple_or_reference Tuple2, 
		typename BinaryOp1, typename BinaryOp2, typename Init>
	constexpr auto tuple_inner_preduct(const Tuple1&& t1, const Tuple2& t2, BinaryOp1 op1, BinaryOp2 op2, Init init)
	{
		constexpr auto size1 = tuple_size_v<remove_cvref_t<Tuple1>>;
		constexpr auto size2 = tuple_size_v<remove_cvref_t<Tuple1>>;
		static_assert(size1 == size2);
		return static_looper<0, size1>::inner_product(t1, t2, std::move(op1), std::move(op2), std::move(init));
	}

	template <tuple_or_reference Tuple, typename... Ts, typename Operation>
	constexpr decltype(auto) tuple_dynamic_set(Tuple& t, Operation op, int idx)
	{
		constexpr auto size = tuple_size_v<Tuple>;
		return static_looper<0, size>::dynamic_set(t, std::move(op), idx);
	}


}


#endif