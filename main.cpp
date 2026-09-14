#include <winnow/winnow.hpp>
#include <leviathan/extc++/all.hpp>
#include <print>
#include <meta>

struct [[=cpp::derive::debug]] Color { int r, g, b; };

int main()
{
    // auto t = std::make_tuple(1, 2, 3, 4, 5);
    // auto color = cpp::make_from_tuple<Color>(cpp::select_tuple_element<1, 2, 3>(t));
    // std::println("color = {}", color);

    // auto c = winnow::stream<winnow::context_error>("[1,2,3, 4, 5, 6, ]");

    // auto result = winnow::universal_parser<std::vector<int>>{}(c);
    // std::println("result = {}", result.value());

    // auto arr = {1, 2, 3, 4, 5,};

    auto c2 = winnow::stream<winnow::context_error>("( 0, True, [3.14, 2.1],  )");
    auto result2 = winnow::universal_parser<std::tuple<int, bool, std::vector<double>>>{}(c2);
    std::println("result2 = {}", result2.value());

}
