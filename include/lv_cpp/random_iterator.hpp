
#include <random>
#include <iterator>
#include <iostream>

namespace leviathan
{

	template <typename Distribution = std::uniform_int_distribution<int>, typename Generator = std::mt19937_64>
	struct distribution_iterator
	{

        static_assert(std::is_same_v<Distribution, std::remove_cv_t<Distribution>>);
        static_assert(std::is_same_v<Generator, std::remove_cv_t<Generator>>);

		using distribution_type = Distribution;
		using generator_type = Generator;

		distribution_iterator() 
			: random_generator{ std::random_device()() }, distribution{ }, value { }
		{
			this->operator++();
			value = this->operator*();
		}

		template <typename Seed, typename... Args>
		distribution_iterator(Seed seed, Args... args)
			: random_generator{ seed }, distribution{ args... }, value { } 
		{
			this->operator++();
			value = this->operator*();
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

        // without this noexpect expr, there are difference behavior between MSVC and GCC
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
			value = distribution(random_generator);
			return *this;
		}

		distribution_iterator operator++(int)
		{
			auto old = *this;
			++* this;
			return old;
		}

		Distribution& get_distribution() noexcept
		{
			return distribution;
		}

		Generator& get_generator() noexcept
		{
			return random_generator;
		}

		constexpr bool operator==(const distribution_iterator&) const noexcept
		{
			return false;
		}

		constexpr bool operator!=(const distribution_iterator&) const noexcept
		{
			return true;
		}

		Generator random_generator;
		Distribution distribution;
		value_type value;
	};


}

// template <typename D, typename G, typename... Args>
// void test1(Args... args)
// {
// 	leviathan::distribution_iterator<D, G> iter{ args... };
// 	for (int i = 0; i < 3; ++i)
// 	{
// 		std::cout << *iter << ' ';
// 		iter++;
// 	}
// 	static_assert(std::forward_iterator<decltype(iter)>);
// 	std::cout << std::is_nothrow_move_constructible_v<decltype(iter)> << std::endl;
// 	std::cout << std::is_nothrow_move_assignable_v<decltype(iter)> << std::endl;
// 	std::endl(std::cout);
// }



// void test()
// {
// 	std::random_device rd;
// 	test1<std::uniform_int_distribution<int>, std::mt19937_64>();
// 	test1<std::normal_distribution<double>, std::mt19937_64>(rd(), 0., 1.);
// 	test1<std::geometric_distribution<>, std::mt19937_64>();
// 	test1<std::gamma_distribution<>, std::mt19937_64>();
// 	test1<std::discrete_distribution<>, std::mt19937_64>();
// }