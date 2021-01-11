#include <iostream>
#include <lv_cpp/ranges/zip.hpp>
#include <fstream>
#include <iterator>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <ranges>
#include <lv_cpp/template_info.hpp>
#include <lv_cpp/output.hpp>

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



#if 1

void test();

void test1();

void test2();


int main()
{
    test1();
    return 0;
}

void test1()
{
    using namespace output;
    std::ifstream is{"./data.txt", std::ios::binary};
    std::istream_iterator<int> initer{is};
    auto sub = std::ranges::subrange(initer, std::istream_iterator<int>());
    
    auto zipper_ = ::leviathan::views::zip(vec, buf, ls, arr, sub, map);
    for (auto [a, b, c, d, e, f] : zipper_ )
    {
        std::cout << a << '-' << b << '-' << 
            c << '-' << d << '-' << e << '-' << f << std::endl;
    }

    // for (auto s : sub)
    // {
    //     std::cout << s << std::endl;
    // }

}

void test()
{
    std::cout << "test\n";
    // std::ranges::range_value_t
    // std::ranges::iterator_t

    // std::cout << "the address of arr[0] is :" << &*arr.begin() << std::endl;    
    
    // auto iter = zip(arr, buf, ls);
    // iter.show();
    // ++iter;
    // iter++;
    // auto a = *iter;
    // // using T2 = decltype(iter)::value_type;
    // // using T1 = decltype(iter)::element_type;
    // std::cout << std::get<0>(a) << '-' << std::get<1>(a) << '-' << std::get<2>(a) << std::endl;
    // std::cout << "the address of first element is :" << &std::get<0>(a) << std::endl;    
    // std::cout << "===================================" << std::endl;    
}



void test2()
{
    // using T = std::tuple<int, double>;
    // using RT = typename add_lvalue_reference<T>::type;
    // int a = 0;
    // double b = 0;
    // RT aa{a, b};
    // PrintTypeCategory(aa);
    // std::cout << "======================================\n";
}
#endif