#include <iostream>
#include <format>
#include <lv_cpp/collections/internal/sorted_list.hpp>
#include <ranges>
#include <vector>
#include <set>
#include <string>

int main(int argc, char const *argv[])
{
    using T = int;
    ::leviathan::collections::sorted_set<T> s;
    std::cout << s.insert(1).second << '\n';
    std::cout << s.insert(3).second << '\n';
    std::cout << s.insert(5).second << '\n';


    static_assert(std::ranges::bidirectional_range<decltype(s)>);

    std::ranges::copy(s, std::ostream_iterator<T>(std::cout, " "));

    assert(s.contains(1));
    assert(!s.contains(2));


    std::cout << '\n';

    std::cout << "Ok\n";
    return 0;
}
