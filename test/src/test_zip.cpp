#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <lv_cpp/template_info.hpp>
#include <lv_cpp/ranges/zip.hpp>

std::vector vec{1, 2, 3, 4, 5};
std::set buf = {6, 7, 8, 9, 10};
const std::list ls{11, 12, 13, 14, 15};
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

int main()
{
    using U1 = typename leviathan::zip_iter_base<T1, T2, T3, T4, T5>::iterator_category;
    using U2 = typename leviathan::zip_iter_base<T1, T2, T3, T4, T5>::reference;
    using U3 = typename leviathan::zip_iter_base<T1, T2, T3, T4, T5>::value_type;
    PrintTypeInfo(U1);
    PrintTypeInfo(U2);
    PrintTypeInfo(U3);
}


#if 0

void test();

void test1();

void test2();


int main()
{
    // auto iter = zip_begin(arr, buf);
    // using T = decltype(iter);
    // std::cout << std::input_iterator<T>;

    auto a = ranges::zip(arr, buf);
    PrintTypeCategory(5);
    return 0;
}

void test1()
{
    using namespace leviathan;
    auto z_b = zip_begin(vec, buf, ls, arr, map);
    auto z_e = zip_end(vec, buf, ls, arr, map);
    int i = 0;
    while (z_b != z_e) {
        std::cout << i++ << std::endl;
        auto t = *z_b;
        PrintTypeCategory(t);
        std::cout << t << std::endl;
        auto [a, b, c, d, e] = t;
        z_b ++;
    }
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