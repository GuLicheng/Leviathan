#include "./../include/zip.hpp"
#include <list>
#include <vector>
#include <set>
#include <map>
#include "./../include/output.hpp"
#include "./../include/template_info.hpp"
#include <range/v3/view/zip.hpp>

std::vector vec{1, 2, 3, 4, 5};
std::set buf = {6, 7, 8, 9, 10};
std::list ls{11, 12, 13, 14, 15};
int arr[] = {16, 17, 18, 19, 20};
std::map<int, int> map =
{
    {-1, -1},
    {-2, -1},
    {-3, -1},
    {-4, -1},
    {-5, -1}
};

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
