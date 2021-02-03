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

template <typename Fn, typename... Ts>
auto for_each_call(Fn f, Ts... ts)
{
    // to avoid optimize, make it complex
    (void)std::initializer_list<int>
	{
        ((void)f(ts), 0)...
    };
	// 	(f(ts), ...);  for c++17
}

/*
    auto for_each_call = [](auto f, auto ... xs) {
        return std::initializer_list<int>{f(xs)...};
    }
1. 其使用f函数的所有调用返回值，构造了一个初始化列表。但我们并不关心返回值。
2. 虽然其返回的初始化列表，但是我们想要一个“即发即弃”的函数，这些函数不用返回任何东西。
3. f在这里可能是一个函数，因为其不会返回任何东西，可能在编译时就会被优化掉。

不返回初始化列表，但会将所有表达式使用(void)std::initializer_list<int>{...}转换为void类型。
初始化表达式中，其将f(xs)...包装进(f(xs),0)...表达式中。这会让程序将返回值完全抛弃，不过0将会放置在初始化列表中。
f(xs)在(f(xs), 0)...表达式中，将会再次转换成void，所以这里就和没有返回值一样。

不推荐使用C风格的类型转换，因为C++有自己的转换操作。
我们可以使用reinterpret_cast<void>(expression)代替例程中的代码行，
不过这样会降低代码的可读性，会给后面的阅读者带来一些困扰。

*/

static auto brace_print(char a, char b) {
    return [=](auto x) {
        std::cout << a << x << b << " ";
    };
}

/*
    auto f = brace_print('(', ')');
    auto g = brace_print('[', ']');
    auto h = brace_print('{', '}');
    auto nl = [](auto) { std::cout << std::endl; };
    auto call_fgh = (multicall(f, g, h, nl));
    for_each_call(call_fgh, 1, 2, 3, 4, 5);
*/

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

/*
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

/*

static void print_pair(int x, int y) {
    std::cout << "(" << x << ", " << y << ")\n";
}


constexpr auto call_cart (
    [=](auto f, auto x, auto ...rest) constexpr {
        (void)std::initializer_list<int>{
            (((x < rest)
                ? (void)f(x, rest)
                : (void)0)
            ,0)...
        };
    });
constexpr auto cartesian = [=](auto... xs) constexpr {
    return [=](auto f) constexpr {
        (void)std::initializer_list<int>{
            ((void)call_cart(f, xs, xs...), 0)...
        };
    };
};

constexpr auto print_cart = cartesian(1, 2, 3, 4);
print_cart(print_pair);

*/

}  // namespace leviathan

#endif