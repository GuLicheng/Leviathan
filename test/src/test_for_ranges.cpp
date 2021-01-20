#include <lv_cpp/ranges/core.hpp>

#include <iostream>
#include <string>

namespace vs = ::leviathan::views;

void print_str_range(auto rg)
{
    std::cout << '(';
    for (auto val : rg)
        std::cout << val;
    
    std::cout << ')';
    std::cout << std::endl;
}

int main()
{
    std::string str = "  hello world   ";

    // trim with drop_last
    auto s1 = str | vs::trim(::isspace);
    print_str_range(s1);

    // take_last_while
    auto s2 = s1 | vs::take_last_while(::isalpha);
    print_str_range(s2);

    // drop_last
    auto s3 = s2 | vs::drop_last(1);
    print_str_range(s3);

    // take_last
    auto s4 = s1 | vs::take_last(5);
    print_str_range(s4);

}