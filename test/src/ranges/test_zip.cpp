#include <iostream>
#include <algorithm>
#include <lv_cpp/ranges/zip.hpp>
#include <fstream>
#include <iterator>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <ranges>
#include <lv_cpp/utils/template_info.hpp>

//  #include <lv_cpp/io/console.hpp>

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& __pair)
{
    return os << '(' << __pair.first << ", " << __pair.second << ')';
}


std::vector vec{1, 2, 3, 4, 5};
std::set buf = {6, 7, 9, 10};
const std::list ls{14, 15};
int arr[] = {16, 17, 18, 19, 20};
std::map<int, int> map =
{
    {-1, -1},
    {-2, -1},
    {-3, -1},
    {-4, -1},
    {-5, -1}
};

using T1 = decltype(vec);
using T2 = decltype(buf);
using T3 = decltype(ls);
using T4 = decltype(arr);
using T5 = decltype(map);

// std::input_interator

void test1();
void test2();

constexpr int test3()
{
    int arr[] = {1, 2, 3};
    int buf[] = {1, 2, 3};
    int res = 0;
    for (auto [a, b] : leviathan::views::zip(arr, buf))
    {
        res += a + b;
    }
    return res;
}

int main()
{
    test1();
    test2();
    // static_assert(12 == test3());
    return 0;
}


void test1()
{
    using ::leviathan::views::zip;
    // using ::leviathan::views::zip0;
    std::ifstream is{"./data.txt", std::ios::binary};
    std::istream_iterator<int> initer{is};
    auto sub = std::ranges::subrange(initer, std::istream_iterator<int>());
    
    auto zipper_ = zip(vec, buf, ls, arr, sub, map);

    std::cout << "Is zip(all...) a input range? " 
        << std::ranges::input_range<decltype(zipper_)> << std::endl;

    for (auto [a, b, c, d, e, f] : zipper_ )
    {
        std::cout << a << '-' << b << '-' << 
            c << '-' << d << '-' << e << '-' << f << std::endl;
        // console::write_line_multi(a, b, c,d, e, f);
    }

    // for (auto s : sub)
    // {
    //     std::cout << s << std::endl;
    // }

    auto view1 = vec | std::views::take(1);
    auto view2 = zip(buf, ls);

    static_assert(std::ranges::bidirectional_range<decltype(view2)> == true);
    static_assert(std::ranges::forward_range<decltype(view1)> == true);

    for (auto val : view1); // test whether it can be compilered
    for (auto val : view2);
    for (auto val : zip(vec | std::views::take(1), buf));

    for (auto val : zip(view1, view2))
    {
        // std::views::print(val);
        auto [a, b] = val;
        // a is int, b is a structure with two attribute
        auto [c, d] = b;
        std::cout << a << '-' << c << '-' << d << std::endl;
    }

    // for this
    // auto rg0 = vec | std::views::take(1) | zip(buf);
    // you can use zip_with or
    // zip(vec | std::views::take(1), buf)

    for (auto val : zip(vec, ls) | std::views::take(2))
    {
        auto [a, b] = val;
        std::cout << a << '-' << b << std::endl;
        // std::cout << val << std::endl;
        // PrintValueCategory(val);
        // std::cout << std::get<0>(val) << '-' << std::get<1>(val) << std::endl;
    }

    auto rr = zip(vec, arr);
    // std::cout << rr.size() << std::endl;
    static_assert(std::ranges::random_access_range<decltype(rr)> == true);

    

    // using T1 = decltype(rr.begin());
    // using T2 = decltype(rr.end());
    // std::cout << std::sized_sentinel_for<T1, T2> << std::endl;
    // std::cout << std::totally_ordered<decltype(rr.begin())> << std::endl;
    // std::cout << std::sentinel_for<T1, T2> << std::endl;
    // std::cout << !std::disable_sized_sentinel_for<std::remove_cv_t<T1>, std::remove_cv_t<T2>> << std::endl;
    // std::cout << _rand<T1> << std::endl;
    auto&& first = std::ranges::begin(rr);
    auto&& last = std::ranges::end(rr);
    using R = std::remove_cvref_t<decltype(first)>::iterator_category;

    PrintTypeInfo(R);

    rr.back();
    rr.front();
    rr.size();
    // rr.data();
    // Since the iterators of contiguous containers in STL are 
    // marked with random_access_iterator_tag
    // the data() cannot be called 
    rr.empty();
    rr[2];

    // contiguous_iterator_tag
    //

// using iterator_category = ::std::common_type_t<
            // typename ::std::iterator_traits<::std::ranges::iterator_t<Rgs>>::iterator_category...>;

    using TT1 = typename ::std::iterator_traits<::std::ranges::iterator_t<T1>>::iterator_category;
    using TT4 = typename ::std::iterator_traits<::std::ranges::iterator_t<T4>>::iterator_category;

    // std::cout << std::ranges::contiguous_range << std::endl;

    PrintTypeInfo(TT1);
    PrintTypeInfo(TT4);

    std::cout << (last - first) << std::endl;
    std::cout << "Test Successfully\n";

/*
template<class S, class I>
  concept sized_sentinel_for =
    std::sentinel_for<S, I> &&
    !std::disable_sized_sentinel_for<std::remove_cv_t<S>, std::remove_cv_t<I>> &&
    requires(const I& i, const S& s) {
      { s - i } -> std::same_as<std::iter_difference_t<I>>;
      { i - s } -> std::same_as<std::iter_difference_t<I>>;
    };
*/

/*
template <typename _Iter>
requires(_Iter __i, const _Iter __j,
		  const iter_difference_t<_Iter> __n)
      {
	{ __i += __n } -> same_as<_Iter&>;
	{ __j +  __n } -> same_as<_Iter>;
	{ __n +  __j } -> same_as<_Iter>;
	{ __i -= __n } -> same_as<_Iter&>;
	{ __j -  __n } -> same_as<_Iter>;
	{  __j[__n]  } -> same_as<iter_reference_t<_Iter>>;
      };
*/

/*
    view_interface

    back()
    front()
    bool()
    size()
    data()
    empty()
    begin()
    end()
    operator[]

*/
}

void test2()
{
    std::cout << "==========================================================\n";
    std::vector vec{3, 2, 1};
    int arr[] = {3, 2, 1};
    auto rg = ::leviathan::views::zip(vec, arr);
    std::ranges::for_each(rg | std::views::reverse, [](auto x)
    {
        auto [a, b] = x;
        std::cout << a << '-' << b << std::endl;
    });
    std::cout << "==========================================================\n";
    // std::ranges::reverse(rg.begin(), rg.end()); not satisfied indirectly_writable
    std::reverse(rg.begin(), rg.end());
    std::ranges::for_each(rg, [](auto x)
    {
        auto [a, b] = x;
        std::cout << a << '-' << b << std::endl;
    });
}

