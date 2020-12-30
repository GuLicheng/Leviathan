#include <iostream>
#include <my_cpp/type_list.hpp>

namespace meta = leviathan::meta;
namespace lv = leviathan;

int main()
{

    static_assert(meta::size<int, double, bool>::value == 3);
    static_assert(meta::size<>::value == 0);

    static_assert(std::is_same_v<meta::front<int, double>::type, int>);
    static_assert(std::is_same_v<meta::front<std::tuple<bool>>::type, bool>);
    static_assert(std::is_same_v<meta::back<int, double>::type, double>);
    static_assert(std::is_same_v<meta::back<std::tuple<bool>>::type, bool>);

    static_assert(meta::empty<>::value == true);
    static_assert(meta::empty<std::tuple<double, bool>>::value == false);
    static_assert(meta::empty<int>::value == false);

    static_assert(std::is_same_v<meta::push_front<std::tuple<int>, double>::type, std::tuple<double, int>>);
    static_assert(std::is_same_v<meta::push_back<std::tuple<int>, double>::type, std::tuple<int, double>>);

    static_assert(std::is_same_v<meta::pop_front<std::tuple<int, double>>::type, std::tuple<double>>);
    static_assert(std::is_same_v<meta::pop_back<std::tuple<int, double>>::type, std::tuple<int>>);

    static_assert(std::is_same_v<meta::max_type<char, int, long long>::type, long long>);
    static_assert(std::is_same_v<meta::min_type<char, int, long long>::type, char>);
    
    static_assert(std::is_same_v<meta::reverse<int, double, bool>::type, meta::reverse<std::tuple<int, double, bool>>::type>);

    static_assert(meta::find_first_index_of<std::tuple<int, double, bool, bool>, bool>::value == 2);

    static_assert(std::is_same_v<meta::insert_sort<int, double, char>::type, meta::insert_sort<std::tuple<char, double, int>>::type>);

    static_assert(std::is_same_v<meta::unique<int, int, double, int>::type, meta::unique<std::tuple<int, double, double>>::type>);

    std::cout << "Test Successfully\n";

}