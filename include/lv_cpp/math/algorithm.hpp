#ifndef __ALGORITHM_HPP__
#define __ALGORITHM_HPP__

#include <algorithm>
#include <vector>


namespace leviathan
{

    struct insertion_sort_fn
    {
        // for simplifier, use random_access_iterator 
        template <std::random_access_iterator I, std::sentinel_for<I> S, 
                    typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<I, Comp, Proj>
        constexpr I operator()(I first, S last, Comp comp = {}, Proj proj = {}) const
        {
            if (first == last)
                return first;

            auto i = first + 1;
            for (; i != last; ++i)
            {
                auto j = i - 1;
                // if arr[j] <= arr[i] continue
                if (!std::invoke(comp, std::invoke(proj, *i), std::invoke(proj, *j))) continue;
                else
                {
                    auto pos = std::ranges::upper_bound(first, i, *i, comp, proj);
                    auto tmp = std::move(*i);
                    std::ranges::move(pos, i, pos + 1);
                    *pos = std::move(tmp);
                } 
            }
            return i;
        }

        template <std::ranges::random_access_range Range, typename Comp = std::ranges::less, typename Proj = std::identity>
        requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj>
        constexpr std::ranges::borrowed_iterator_t<Range>
        operator()(Range&& r, Comp comp = {}, Proj proj = {}) const
        {
            return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(comp), std::move(proj));
        }
    };

    inline constexpr insertion_sort_fn insert_sort{ };


template <typename InIt, typename OutIt, typename T, typename F>
InIt split(InIt it, InIt end_it, OutIt out_it, T split_val, F bin_func) 
{
	using namespace std;
	while (it != end_it) 
    {
		auto slice_end(find(it, end_it, split_val));
		*out_it++ = bin_func(it, slice_end);
		if (slice_end == end_it) 
        {
			return end_it;
		}
		it = next(slice_end);
	}
	return it;
}



template <typename T, typename... Ts>
auto concat(T&& t, Ts&&... ts)
{
	if constexpr (sizeof...(ts) > 0)
	{
		return [=](auto... paras)
		{
			return t(concat(ts...)(paras...));
		};
	}
	else
	{
		return [=](auto... paras)
		{
			return t(paras...);
		};
	}
	// f(g(h(x, y, z, ...))), only accept functions
}

template <typename A, typename B, typename Fn>
auto combine(Fn binary_func, A&& a, B&& b)
{
	return [=](auto param)
	{
		return binary_func(a(param), b(param));
	};
}

/*
bool begin_with_a(const std::string& s) {
    std::cout << s << std::endl;
    return s.find_first_of('a') == 0;
}
bool end_with_b(const std::string& s) {
    std::cout << s << std::endl;
    return s.find_last_of('b') == s.length() - 1;
}
auto a_XXX_b = combine(std::logical_and<>{}, begin_with_a, end_with_b);
std::cout << a_XXX_b("aaabbb") << std::endl;
*/

/*

template <typename T>
auto map(T fn) {
    return [=](auto reduce_fn) {
        return [=] (auto accum, auto input) {
            return reduce_fn(accum, fn(input));
        };
    };
}

template <typename T>
auto filter(T predicate) {
    return [=](auto reduce_fn) {
        return [=](auto accume, auto input) {
            if (predicate(input)) {
                return reduce_fn(accume, input);
            } else {
                return accume;
            }
        };
    };
}


    std::istream_iterator<int> it{ std::cin };
    std::istream_iterator<int> end_it;
    auto even = [](int x) { return ~x & 1; };
    auto twice = [](int x) { return x << 1; };
    auto copy_and_advance = [](auto iter, auto input) {
        *iter = input;
        return ++iter;
    };
	// very ugly
    std::accumulate(it, end_it, std::ostream_iterator<int>{ std::cout, ", " }, 
        filter(even) (
            map(twice) (
                copy_and_advance
            )
        ));
    std::cout << std::endl;
*/

}  // namespace leviathan

#endif