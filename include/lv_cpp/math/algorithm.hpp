#ifndef __ALGORITHM_HPP__
#define __ALGORITHM_HPP__

#include <algorithm>
#include <vector>

#define __USELESS__ 0

namespace leviathan
{
#if __USELESS__
template <typename RandomAccessIter, typename Sentinel, typename BinaryOp>
/* constexpr */ inline size_t 
longest_sub_sequence(RandomAccessIter first, Sentinel last, BinaryOp cmp)
{
    if (first == last)
        return 0;
    if (first + 1 == last)
        return 1;
    std::vector<size_t> res(last - first, 1);

    for (auto right = first; right != last; ++right)
    {
        for (auto left = first; left != right; ++left)
        {
            if (cmp(*right, *left))
            {
                const auto index_r = right - first;
                const auto index_l = left - first;
                res[index_r] = max(res[index_r], res[index_l] + 1);
            }
        }
    }
    return *max_element(res.begin(), res.end());
}
#endif

/*
    https://leetcode-cn.com/problems/longest-increasing-subsequence/submissions/
    https://www.acwing.com/problem/content/484/
    longset_sub_sequence ascend -> cmp = less
    longset_sub_sequence not_ascend -> cmp = less_equal
    longset_sub_sequence descend -> cmp = greater
    longset_sub_sequence not_descend -> cmp = greater_equal
*/
template <typename RandomAccessIter, typename Sentinel, typename BinaryOp>
/* constexpr */ inline size_t 
longest_sub_sequence(RandomAccessIter first, Sentinel last, BinaryOp cmp)
{
    if (first == last)
        return 0;
    if (first + 1 == last)
        return 1;
    
    std::vector<typename std::iterator_traits<RandomAccessIter>::value_type> _stack{*first};
    _stack.reserve((last - first) >> 1);
    for (auto iter = first + 1; iter != last; ++iter)
    {
        if (cmp(_stack.back(), *iter))
        {
            _stack.emplace_back(*iter);
        }
        else
        {
            *std::lower_bound(_stack.begin(), _stack.end(), *iter, cmp) = *iter;
        }        
    }
    return _stack.size();
}


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


template <typename BidirectionalIterator>
constexpr auto find_most_frequently(BidirectionalIterator first, BidirectionalIterator last)
{
    // assert(std::is_sorted(first, last))
    if (first == last)
        throw std::runtime_error("Empty range is illegal");
    BidirectionalIterator left = first;
    BidirectionalIterator right = left;
    auto value = *left;
    typename std::iterator_traits<BidirectionalIterator>::difference_type max_dist = 0;
    typename std::iterator_traits<BidirectionalIterator>::difference_type d;
    while (left != last)
    {
        auto val = *left;
        std::cout << val << '\n';
        auto iter = left + 1;
        for (d = 0; iter != last && *iter == val; ++iter, ++d);
        // update
        if (d > max_dist)
        {
            max_dist = d;
            value = val;
        }
        left = iter;
    }
    return value;
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

template <typename... Ts>
auto multicall(Ts... functions)
{
	return [=](auto x)
	{
		(void)std::initializer_list<int>
		{
			((void)functions(x), 0)...;
		};
	};
}

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