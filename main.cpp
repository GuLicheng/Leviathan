#include <iostream>
#include <print>
#include <meta>

consteval std::string_view calc_type_name(std::meta::info info)
{
    using T = std::int8_t;
    if (dealias(info) == dealias(^^T)) return "int8";
    return display_string_of(info);
}

int main()
{
    std::cout << calc_type_name(^^int) << std::endl;
    std::cout << calc_type_name(^^::int8_t) << std::endl;

    return 0;
}