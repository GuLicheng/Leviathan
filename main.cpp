#include <winnow/winnow.hpp>
#include <leviathan/extc++/all.hpp>
#include <print>
#include <meta>


int main()
{
    constexpr auto info = std::meta::substitute( ^^std::tuple, { ^^int, ^^double } );
    using T = typename [:info:];
    T t = { 1, 3.14 };
    std::println("{}", t);
}
