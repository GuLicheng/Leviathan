
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include <lv_cpp/catch.hpp>
#include "expected.hpp"

TEST_CASE("Simple assignment", "[assignment.simple]")
{
	std::expected<int, int> e1 = 42;
	std::expected<int, int> e2 = 17;
	std::expected<int, int> e3 = 21;
	std::expected<int, int> e4 = std::make_unexpected(42);
	std::expected<int, int> e5 = std::make_unexpected(17);
	std::expected<int, int> e6 = std::make_unexpected(21);

	e1 = e2;
	REQUIRE(e1);
	REQUIRE(*e1 == 17);
	REQUIRE(e2);
	REQUIRE(*e2 == 17);

	e1 = std::move(e2);
	REQUIRE(e1);
	REQUIRE(*e1 == 17);
	REQUIRE(e2);
	REQUIRE(*e2 == 17);

	e1 = 42;
	REQUIRE(e1);
	REQUIRE(*e1 == 42);

	auto unex = std::make_unexpected(12);
	e1 = unex;
	REQUIRE(!e1);
	REQUIRE(e1.error() == 12);

	e1 = std::make_unexpected(42);
	REQUIRE(!e1);
	REQUIRE(e1.error() == 42);

	e1 = e3;
	REQUIRE(e1);
	REQUIRE(*e1 == 21);

	e4 = e5;
	REQUIRE(!e4);
	REQUIRE(e4.error() == 17);

	e4 = std::move(e6);
	REQUIRE(!e4);
	REQUIRE(e4.error() == 21);

	e4 = e1;
	REQUIRE(e4);
	REQUIRE(*e4 == 21);
}

TEST_CASE("Assignment deletion", "[assignment.deletion]")
{
	struct has_all
	{
		has_all() = default;
		has_all(const has_all &) = default;
		has_all(has_all &&) noexcept = default;
		has_all &operator=(const has_all &) = default;
	};

	std::expected<has_all, has_all> e1 = {};
	std::expected<has_all, has_all> e2 = {};
	e1 = e2;

	struct except_move
	{
		except_move() = default;
		except_move(const except_move &) = default;
		except_move(except_move &&) noexcept(false){};
		except_move &operator=(const except_move &) = default;
	};
	std::expected<except_move, except_move> e3 = {};
	std::expected<except_move, except_move> e4 = {};
	// e3 = e4; should not compile
}

TEST_CASE("Triviality", "[bases.triviality]")
{
	REQUIRE(std::is_trivially_copy_constructible<std::expected<int, int>>::value);
	REQUIRE(std::is_trivially_copy_assignable<std::expected<int, int>>::value);
	REQUIRE(std::is_trivially_move_constructible<std::expected<int, int>>::value);
	REQUIRE(std::is_trivially_move_assignable<std::expected<int, int>>::value);
	REQUIRE(std::is_trivially_destructible<std::expected<int, int>>::value);

	// REQUIRE(std::is_trivially_copy_constructible<std::expected<void,int>>::value);
	// REQUIRE(std::is_trivially_move_constructible<std::expected<void,int>>::value);
	// REQUIRE(std::is_trivially_destructible<std::expected<void,int>>::value);

	{
		struct T
		{
			T(const T &) = default;
			T(T &&) = default;
			T &operator=(const T &) = default;
			T &operator=(T &&) = default;
			~T() = default;
		};
		REQUIRE(std::is_trivially_copy_constructible<std::expected<T, int>>::value);
		REQUIRE(std::is_trivially_copy_assignable<std::expected<T, int>>::value);
		REQUIRE(std::is_trivially_move_constructible<std::expected<T, int>>::value);
		REQUIRE(std::is_trivially_move_assignable<std::expected<T, int>>::value);
		REQUIRE(std::is_trivially_destructible<std::expected<T, int>>::value);
	}

	{
		struct T
		{
			T(const T &) {}
			T(T &&){};
			T &operator=(const T &) { return *this; }
			T &operator=(T &&) { return *this; };
			~T() {}
		};
		REQUIRE(!std::is_trivially_copy_constructible<std::expected<T, int>>::value);
		REQUIRE(!std::is_trivially_copy_assignable<std::expected<T, int>>::value);
		REQUIRE(!std::is_trivially_move_constructible<std::expected<T, int>>::value);
		REQUIRE(!std::is_trivially_move_assignable<std::expected<T, int>>::value);
		REQUIRE(!std::is_trivially_destructible<std::expected<T, int>>::value);
	}
}

TEST_CASE("Deletion", "[bases.deletion]")
{
	REQUIRE(std::is_copy_constructible<std::expected<int, int>>::value);
	REQUIRE(std::is_copy_assignable<std::expected<int, int>>::value);
	REQUIRE(std::is_move_constructible<std::expected<int, int>>::value);
	REQUIRE(std::is_move_assignable<std::expected<int, int>>::value);
	REQUIRE(std::is_destructible<std::expected<int, int>>::value);

	{
		struct T
		{
			T() = default;
		};
		REQUIRE(std::is_default_constructible<std::expected<T, int>>::value);
	}

	{
		struct T
		{
			T(int) {}
		};
		REQUIRE(!std::is_default_constructible<std::expected<T, int>>::value);
	}

	{
		struct T
		{
			T(const T &) = default;
			T(T &&) = default;
			T &operator=(const T &) = default;
			T &operator=(T &&) = default;
			~T() = default;
		};
		REQUIRE(std::is_copy_constructible<std::expected<T, int>>::value);
		REQUIRE(std::is_copy_assignable<std::expected<T, int>>::value);
		REQUIRE(std::is_move_constructible<std::expected<T, int>>::value);
		REQUIRE(std::is_move_assignable<std::expected<T, int>>::value);
		REQUIRE(std::is_destructible<std::expected<T, int>>::value);
	}

	{
		struct T
		{
			T(const T &) = delete;
			T(T &&) = delete;
			T &operator=(const T &) = delete;
			T &operator=(T &&) = delete;
		};
		REQUIRE(!std::is_copy_constructible<std::expected<T, int>>::value);
		REQUIRE(!std::is_copy_assignable<std::expected<T, int>>::value);
		REQUIRE(!std::is_move_constructible<std::expected<T, int>>::value);
		REQUIRE(!std::is_move_assignable<std::expected<T, int>>::value);
	}

	{
		struct T
		{
			T(const T &) = delete;
			T(T &&) = default;
			T &operator=(const T &) = delete;
			T &operator=(T &&) = default;
		};
		REQUIRE(!std::is_copy_constructible<std::expected<T, int>>::value);
		REQUIRE(!std::is_copy_assignable<std::expected<T, int>>::value);
		REQUIRE(std::is_move_constructible<std::expected<T, int>>::value);
		REQUIRE(std::is_move_assignable<std::expected<T, int>>::value);
	}

	{
		struct T
		{
			T(const T &) = default;
			T(T &&) = delete;
			T &operator=(const T &) = default;
			T &operator=(T &&) = delete;
		};
		REQUIRE(std::is_copy_constructible<std::expected<T, int>>::value);
		REQUIRE(std::is_copy_assignable<std::expected<T, int>>::value);
	}

	{
		std::expected<int, int> e;
		REQUIRE(std::is_default_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_constructible<decltype(e)>::value);
		REQUIRE(std::is_move_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_assignable<decltype(e)>::value);
		REQUIRE(std::is_move_assignable<decltype(e)>::value);
		REQUIRE(std::is_trivially_move_constructible<decltype(e)>::value);
		REQUIRE(std::is_trivially_move_assignable<decltype(e)>::value);
	}

	{
		std::expected<int, std::string> e;
		REQUIRE(std::is_default_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_constructible<decltype(e)>::value);
		REQUIRE(std::is_move_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_assignable<decltype(e)>::value);
		REQUIRE(std::is_move_assignable<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_constructible<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_assignable<decltype(e)>::value);
	}

	{
		std::expected<std::string, int> e;
		REQUIRE(std::is_default_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_constructible<decltype(e)>::value);
		REQUIRE(std::is_move_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_assignable<decltype(e)>::value);
		REQUIRE(std::is_move_assignable<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_constructible<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_assignable<decltype(e)>::value);
	}

	{
		std::expected<std::string, std::string> e;
		REQUIRE(std::is_default_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_constructible<decltype(e)>::value);
		REQUIRE(std::is_move_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_assignable<decltype(e)>::value);
		REQUIRE(std::is_move_assignable<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_constructible<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_assignable<decltype(e)>::value);
	}
}

struct takes_init_and_variadic
{
	std::vector<int> v;
	std::tuple<int, int> t;
	template <class... Args>
	takes_init_and_variadic(std::initializer_list<int> l, Args &&...args)
		: v(l), t(std::forward<Args>(args)...) {}
};

TEST_CASE("Constructors", "[constructors]")
{
	{
		std::expected<int, int> e;
		REQUIRE(e);
		REQUIRE(e == 0);
	}

	{
		std::expected<int, int> e = std::make_unexpected(0);
		REQUIRE(!e);
		REQUIRE(e.error() == 0);
	}

	{
		std::expected<int, int> e(std::unexpect, 0);
		REQUIRE(!e);
		REQUIRE(e.error() == 0);
	}

	{
		std::expected<int, int> e(std::in_place, 42);
		REQUIRE(e);
		REQUIRE(e == 42);
	}

	// {
	//   std::expected<std::vector<int>, int> e(std::in_place, {0, 1}); initializer_list<T> not supported
	//   REQUIRE(e);
	//   REQUIRE((*e)[0] == 0);
	//   REQUIRE((*e)[1] == 1);
	// }

	{
		std::expected<std::tuple<int, int>, int> e(std::in_place, 0, 1);
		REQUIRE(e);
		REQUIRE(std::get<0>(*e) == 0);
		REQUIRE(std::get<1>(*e) == 1);
	}

	// {
	//   std::expected<takes_init_and_variadic, int> e(std::in_place, {0, 1}, 2, 3);
	//   REQUIRE(e);
	//   REQUIRE(e->v[0] == 0);
	//   REQUIRE(e->v[1] == 1);
	//   REQUIRE(std::get<0>(e->t) == 2);
	//   REQUIRE(std::get<1>(e->t) == 3);
	// }

	{
		std::expected<int, int> e;
		REQUIRE(std::is_default_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_constructible<decltype(e)>::value);
		REQUIRE(std::is_move_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_assignable<decltype(e)>::value);
		REQUIRE(std::is_move_assignable<decltype(e)>::value);
		REQUIRE(std::is_trivially_move_constructible<decltype(e)>::value);
		REQUIRE(std::is_trivially_move_assignable<decltype(e)>::value);
	}

	{
		std::expected<int, std::string> e;
		REQUIRE(std::is_default_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_constructible<decltype(e)>::value);
		REQUIRE(std::is_move_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_assignable<decltype(e)>::value);
		REQUIRE(std::is_move_assignable<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_constructible<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_assignable<decltype(e)>::value);
	}

	{
		std::expected<std::string, int> e;
		REQUIRE(std::is_default_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_constructible<decltype(e)>::value);
		REQUIRE(std::is_move_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_assignable<decltype(e)>::value);
		REQUIRE(std::is_move_assignable<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_constructible<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_assignable<decltype(e)>::value);
	}

	{
		std::expected<std::string, std::string> e;
		REQUIRE(std::is_default_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_constructible<decltype(e)>::value);
		REQUIRE(std::is_move_constructible<decltype(e)>::value);
		REQUIRE(std::is_copy_assignable<decltype(e)>::value);
		REQUIRE(std::is_move_assignable<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_constructible<decltype(e)>::value);
		REQUIRE(!std::is_trivially_move_assignable<decltype(e)>::value);
	}

	{
		// std::expected<void, int> e;
		// REQUIRE(e);
	}

	{
		// std::expected<void, int> e(std::unexpect, 42); void is in TODO:
		// REQUIRE(!e);
		// REQUIRE(e.error() == 42);
	}
}

struct move_detector
{
	move_detector() = default;
	move_detector(move_detector &&rhs) { rhs.been_moved = true; }
	bool been_moved = false;
};

TEST_CASE("Observers", "[observers]")
{
	std::expected<int, int> o1 = 42;
	std::expected<int, int> o2{std::unexpect, 0};
	const std::expected<int, int> o3 = 42;

	REQUIRE(*o1 == 42);
	REQUIRE(*o1 == o1.value());
	REQUIRE(o2.value_or(42) == 42);
	REQUIRE(o2.error() == 0);
	REQUIRE(o3.value() == 42);
	auto success = std::is_same<decltype(o1.value()), int &>::value;
	REQUIRE(success);
	success = std::is_same<decltype(o3.value()), const int &>::value;
	REQUIRE(success);
	success = std::is_same<decltype(std::move(o1).value()), int &&>::value;
	REQUIRE(success);

#ifndef TL_EXPECTED_NO_CONSTRR
	success = std::is_same<decltype(std::move(o3).value()), const int &&>::value;
	REQUIRE(success);
#endif

	std::expected<move_detector, int> o4{std::in_place};
	move_detector o5 = std::move(o4).value();
	REQUIRE(o4->been_moved);
	REQUIRE(!o5.been_moved);
}

struct no_throw
{
	no_throw(std::string i) : i(i) {}
	std::string i;
};
struct canthrow_move
{
	canthrow_move(std::string i) : i(i) {}
	canthrow_move(canthrow_move const &) = default;
	canthrow_move(canthrow_move &&other) noexcept(false) : i(other.i) {}
	canthrow_move &operator=(canthrow_move &&) = default;
	std::string i;
};

bool should_throw = false;
struct willthrow_move
{
	willthrow_move(std::string i) : i(i) {}
	willthrow_move(willthrow_move const &) = default;
	willthrow_move(willthrow_move &&other) : i(other.i)
	{
		if (should_throw)
			throw 0;
	}
	willthrow_move &operator=(willthrow_move &&) = default;
	std::string i;
};
static_assert(std::is_swappable<no_throw>::value, "");

// not satisfied constraint (is_nothrow_constructible_v<T, U> || is_nothrow_move_constructible_v<E>) is true
// template <class T1, class T2>
// void swap_test()
// {
// 	std::string s1 = "abcdefghijklmnopqrstuvwxyz";
// 	std::string s2 = "zyxwvutsrqponmlkjihgfedcba";

// 	std::expected<T1, T2> a{s1};
// 	std::expected<T1, T2> b{s2};
// 	swap(a, b);
// 	REQUIRE(a->i == s2);
// 	REQUIRE(b->i == s1);

// 	a = s1;
// 	b = std::unexpected<T2>(s2);
// 	swap(a, b);
// 	REQUIRE(a.error().i == s2);
// 	REQUIRE(b->i == s1);

// 	a = std::unexpected<T2>(s1);
// 	b = s2;
// 	swap(a, b);
// 	REQUIRE(a->i == s2);
// 	REQUIRE(b.error().i == s1);

// 	a = std::unexpected<T2>(s1);
// 	b = std::unexpected<T2>(s2);
// 	swap(a, b);
// 	REQUIRE(a.error().i == s2);
// 	REQUIRE(b.error().i == s1);

// 	a = s1;
// 	b = s2;
// 	a.swap(b);
// 	REQUIRE(a->i == s2);
// 	REQUIRE(b->i == s1);

// 	a = s1;
// 	b = std::unexpected<T2>(s2);
// 	a.swap(b);
// 	REQUIRE(a.error().i == s2);
// 	REQUIRE(b->i == s1);

// 	a = std::unexpected<T2>(s1);
// 	b = s2;
// 	a.swap(b);
// 	REQUIRE(a->i == s2);
// 	REQUIRE(b.error().i == s1);

// 	a = std::unexpected<T2>(s1);
// 	b = std::unexpected<T2>(s2);
// 	a.swap(b);
// 	REQUIRE(a.error().i == s2);
// 	REQUIRE(b.error().i == s1);
// }

// TEST_CASE("swap")
// {

// 	swap_test<no_throw, no_throw>();
// 	swap_test<no_throw, canthrow_move>();
// 	swap_test<canthrow_move, no_throw>();

// 	std::string s1 = "abcdefghijklmnopqrstuvwxyz";
// 	std::string s2 = "zyxwvutsrqponmlkjihgfedcbaxxx";
// 	std::expected<no_throw, willthrow_move> a{s1};
// 	std::expected<no_throw, willthrow_move> b{std::unexpect, s2};
// 	should_throw = 1;

// #ifdef _MSC_VER
// 	// this seems to break catch on GCC and Clang
// 	REQUIRE_THROWS(swap(a, b));
// #endif

// 	REQUIRE(a->i == s1);
// 	REQUIRE(b.error().i == s2);
// }

#include <memory>
#include <vector>
#include <tuple>

struct takes_init_and_variadic2
{
	std::vector<int> v;
	std::tuple<int, int> t;
	template <class... Args>
	takes_init_and_variadic2(std::initializer_list<int> l, Args &&...args)
		: v(l), t(std::forward<Args>(args)...) {}
};

TEST_CASE("Emplace", "[emplace]")
{
	{
		std::expected<std::unique_ptr<int>, int> e;
		e.emplace(new int{42});
		REQUIRE(e);
		REQUIRE(**e == 42);
	}

	// {
	//     std::expected<std::vector<int>,int> e;
	//     e.emplace({0,1});
	//     REQUIRE(e);
	//     REQUIRE((*e)[0] == 0);
	//     REQUIRE((*e)[1] == 1);
	// }

	{
		std::expected<std::tuple<int, int>, int> e;
		e.emplace(2, 3);
		REQUIRE(e);
		REQUIRE(std::get<0>(*e) == 2);
		REQUIRE(std::get<1>(*e) == 3);
	}

	// {
	//     std::expected<takes_init_and_variadic2,int> e = std::make_unexpected(0);
	//     e.emplace({0,1}, 2, 3);
	//     REQUIRE(e);
	//     REQUIRE(e->v[0] == 0);
	//     REQUIRE(e->v[1] == 1);
	//     REQUIRE(std::get<0>(e->t) == 2);
	//     REQUIRE(std::get<1>(e->t) == 3);
	// }
}
