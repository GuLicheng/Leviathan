#include <iostream>
#include <lv_cpp/ranges/zip.hpp>
#include <fstream>
#include <iterator>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <ranges>
#include <lv_cpp/tools/template_info.hpp>
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



void test1();

int main()
{
    test1();
    return 0;
}

void test1()
{
    using namespace output;
    using ::leviathan::views::zip;
    std::ifstream is{"./data.txt", std::ios::binary};
    std::istream_iterator<int> initer{is};
    auto sub = std::ranges::subrange(initer, std::istream_iterator<int>());
    
    auto zipper_ = zip(vec, buf, ls, arr, sub, map);
    for (auto [a, b, c, d, e, f] : zipper_ )
    {
        std::cout << a << '-' << b << '-' << 
            c << '-' << d << '-' << e << '-' << f << std::endl;
    }

    // for (auto s : sub)
    // {
    //     std::cout << s << std::endl;
    // }

    auto view1 = vec | std::views::take(1);
    auto view2 = zip(buf, ls);

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

}

