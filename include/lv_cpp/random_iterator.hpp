module;

#include <random>
#include <iterator>
#include <iostream>
#include <ranges>
#include <concepts>

export module random_iterator;

export namespace leviathan
{

	struct distribution_sentinel { };

	template <typename Distribution = std::uniform_int_distribution<int>, typename Generator = std::mt19937_64>
		struct distribution_iterator
	{
	private:

		static_assert(std::same_as<Distribution, std::remove_cvref_t<Distribution>>);
		static_assert(std::same_as<Generator, std::remove_cvref_t<Generator>>);

		void next_random_number()
		{
			value = distribution(random_generator);
		}

	public:
		using distribution_type = Distribution;
		using generator_type = Generator;

		distribution_iterator()
			: random_generator{ std::random_device()() }, distribution{ }, value{ }
		{
			next_random_number();
		}

		template <typename Seed, typename... Args>
		explicit distribution_iterator(Seed seed, Args... args)
			: random_generator{ seed }, distribution{ args... }, value{ }
		{
			next_random_number();
		}

		distribution_iterator(const distribution_iterator&)
			noexcept(noexcept(std::is_nothrow_copy_constructible_v<distribution_type>
				&& std::is_nothrow_copy_constructible_v<generator_type>)) = default;

		distribution_iterator& operator=(const distribution_iterator&)
			noexcept(noexcept(std::is_nothrow_assignable_v<distribution_type, const distribution_type&>
				&& std::is_nothrow_assignable_v<generator_type, const generator_type&>)) = default;

		distribution_iterator(distribution_iterator&&)
			noexcept(noexcept(std::is_nothrow_move_constructible_v<distribution_type>
				&& std::is_nothrow_move_constructible_v<generator_type>)) = default;
		distribution_iterator& operator=(distribution_iterator&&)
			noexcept(noexcept(std::is_nothrow_move_assignable_v<distribution_type>
				&& std::is_nothrow_move_assignable_v<generator_type>)) = default;


		using value_type = typename distribution_type::result_type;
		using reference = value_type;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;

		reference operator*() const
		{
			return value;
		}

		distribution_iterator& operator++()
		{
			next_random_number();
			return *this;
		}

		distribution_iterator operator++(int)
		{
			auto old = *this;
			++* this;
			return old;
		}

		Distribution& get_distribution()
		{
			return distribution;
		}

		Generator& get_generator()
		{
			return random_generator;
		}

		bool operator==(const distribution_iterator&) const noexcept
		{
			return false;
		}

		bool operator==(const distribution_sentinel&) const noexcept
		{
			return false;
		}

		bool operator!=(const distribution_iterator&) const noexcept
		{
			return true;
		}

		bool operator!=(const distribution_sentinel&) const noexcept
		{
			return true;
		}

		Generator random_generator;
		Distribution distribution;
		value_type value;
	};

	//export template <typename D = std::uniform_int_distribution<int>, typename G = std::mt19937_64>
	//	auto random_range(distribution_iterator<D, G> dist_iter, int count)
	//{
	//	return std::ranges::subrange(dist_iter, distribution_sentinel())
	//		| std::views::take(count);
	//}

	struct random_range_fn
	{
		template <typename D = std::uniform_int_distribution<int>, typename G = std::mt19937_64>
		auto operator()(distribution_iterator<D, G> dist_iter, int count) const
		{
			return std::ranges::subrange(dist_iter, distribution_sentinel())
				| std::views::take(count);
		}
	};

	constexpr random_range_fn random_range{};

} // end of namespace

template <typename D, typename G, typename... Args>
void test1(Args... args)
{
	leviathan::distribution_iterator<D, G> iter{ args... };
	using T = typename leviathan::distribution_iterator<D, G>::value_type;
	auto range5 = leviathan::random_range(iter, 10);
	std::ranges::copy(range5, std::ostream_iterator<T>{std::cout, " "});
	std::endl(std::cout);

	static_assert(std::forward_iterator<decltype(iter)>);
	static_assert(std::ranges::forward_range<decltype(range5)>);
}


export void test()
{
	std::random_device rd;
	test1<std::uniform_int_distribution<int>, std::mt19937_64>(0, 1, 5);
	test1<std::normal_distribution<double>, std::mt19937_64>(rd(), 0., 1.);
	test1<std::geometric_distribution<>, std::mt19937_64>();
	test1<std::gamma_distribution<>, std::mt19937_64>();
	test1<std::discrete_distribution<>, std::mt19937_64>();
}