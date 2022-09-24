#include <assert.h>
#include <type_traits>
#include <iostream>
#include <algorithm>

template <typename... Ts> class TupleImpl;

template <> class TupleImpl <> { };

template <typename Head, typename... Tails> 
class TupleImpl<Head, Tails...>
{
	Head m_head;
	TupleImpl<Tails...> m_tails;

public:	

	constexpr TupleImpl(const TupleImpl&) = default;
	constexpr TupleImpl(TupleImpl&&) = default;

	constexpr TupleImpl(const Head& head, const Tails&... tails)
		: m_head(head), m_tails(tails...) { }


	template <std::size_t N>
	auto& GetElement() &
	{
		if constexpr (N == 0)
			return m_head;
		else
			return m_tails.template GetElement<N - 1>();
	}

	template <std::size_t N>
	auto& GetElement() const&
	{
		if constexpr (N == 0)
			return m_head;
		else
			return m_tails.template GetElement<N - 1>();
	}

	template <std::size_t N>
	auto&& GetElement() &&
	{
		if constexpr (N == 0)
			return std::move(m_head);
		else
			return std::move(m_tails).template GetElement<N - 1>();
	}

	template <std::size_t N>
	auto&& GetElement() const&&
	{
		if constexpr (N == 0)
			return std::move(m_head);
		else
			return std::move(m_tails).template GetElement<N - 1>();
	}

};

template <typename... Types>
class Tuple : public TupleImpl<Types...>
{
	using base = TupleImpl<Types...>;
public:
	// FIXME
	constexpr explicit(true) Tuple() requires (std::is_default_constructible_v<Types> && ...) = default; 

	constexpr explicit(!std::conjunction_v<std::is_convertible<const Types&, Types>...>) Tuple(const Types&... ts) 
		requires (sizeof...(Types) >= 1 and (std::is_copy_constructible_v<Types> && ...))
			: base(ts...) { }

	constexpr Tuple(const Tuple&) = default;
	constexpr Tuple(Tuple&&) = default;
};

template <typename... Types>
class Variant
{
	static_assert(sizeof...(Types) > 0);
	
	static constexpr size_t variant_npos = -1;

	constexpr static auto buffer_size = []() {
		std::size_t sz[] = { sizeof...(Types) };
		return std::ranges::max(sz);
	}();

	alignas(Types...) unsigned char m_raw_buffer[buffer_size];

};


void test() {

	Tuple<int, double, bool> t(1, 3.14, false);

	std::cout << t.GetElement<0>() << '\n';
	std::cout << t.GetElement<1>() << '\n';
	std::cout << t.GetElement<2>() << '\n';

  static_assert(std::is_trivially_copyable_v<Tuple<int, double>>);
  static_assert(std::is_nothrow_move_constructible_v<Tuple<int, double>>);
}


int main()
{
    test();

	Variant<int, double, bool> v;
}
