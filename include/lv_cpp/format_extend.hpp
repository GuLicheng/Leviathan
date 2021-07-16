#ifdef __cpp_lib_format
#include <format>

template <typename Container>
concept ContainerChecker = std::ranges::range<Container>;
// tuple - like
template <typename Tuple>
concept TupleChecker = requires (const Tuple & t)
{
	std::tuple_size<Tuple>::type;
};

template <template <typename...> typename Container, typename... Types, typename CharT>
struct std::formatter<Container<Types...>, CharT>
	: std::formatter<char, CharT>
{
	template <typename FormatContext>
	auto format(const Container<Types...>& rg, FormatContext& format_context)
	{
		using _Ty = Container<Types...>;
		if constexpr (ContainerChecker<_Ty>)
		{
			return format_container(rg, format_context);
		}
		else if constexpr (TupleChecker<_Ty>)
		{
			return format_tuple(rg, format_context);
		}
		else
			static_assert(false, "Not support!");
	}

	template <typename FormatContext>
	auto format_container(const Container<Types...>& rg, FormatContext& format_context)
	{
		using value_type = std::ranges::range_value_t<Container<Types...>>;
		auto iter = std::formatter<char, CharT>().format('[', format_context);
		auto begin = std::begin(rg);
		auto end = std::end(rg);
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

	template <typename FormatContext>
	auto format_tuple(const Container<Types...>& t, FormatContext& format_context)
	{
		auto __print_tuple = [&]<typename _Tuple, size_t... Idx>(const _Tuple & _t, std::index_sequence<Idx...>)
		{
			auto iter = std::formatter<char, CharT>().format('(', format_context);

			auto write = [&]<typename U>(const U & val, bool is_first)
			{
				if (is_first)
					iter = ',', iter = ' ';
				iter = std::formatter<U, CharT>().format(val, format_context);
			};

			(
				(Idx == 0 ? write(::std::get<0>(_t), false) : write(::std::get<Idx>(_t), true))
			, ...);

			iter = ')';
			return iter;
		};

		return __print_tuple(t, std::make_index_sequence<sizeof...(Types)>());
	}
};
#endif

/*
int main()
{
	std::vector arr{ 1, 2, 3 };
	std::list ls{ 4, 5, 6 };
	std::set s{ 7, 8, 9 };
	auto tuple = std::make_tuple(1, 2, 3);
	std::string str = "hello world";
	std::cout << std::format
	("The vec = {}\nls = {}\nset = {}\nstr = {}\ntuple = {}\nchars = {}"
		, arr, ls, s, str, tuple, 0);

	return 0;
}
*/