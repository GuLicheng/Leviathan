#include <winnow/winnow.hpp>
#include <leviathan/extc++/all.hpp>
#include <print>
#include <meta>

struct [[=cpp::derive::debug]] Color { int r, g, b; };


int main()
{
    auto t = std::make_tuple(1, 2, 3, 4, 5);
    auto color = cpp::make_from_tuple<Color>(cpp::select_tuple_element<1, 2, 3>(t));
    std::println("color = {}", color);

    auto c = winnow::stream<winnow::context_error>("[1, 2, 3, 4, 5]");

    auto result = winnow::universal_parser<std::vector<int>>{}(c);
    std::println("result = {}", result.value());
}
