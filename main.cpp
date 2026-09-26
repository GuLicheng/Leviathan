#include <winnow/winnow.hpp>
#include <leviathan/extc++/all.hpp>
#include <print>
#include <meta>

int& func(int a);

int main()
{
    constexpr auto info = std::meta::invoke_result(^^decltype(func), { ^^int });
    constexpr auto type = display_string_of(info);
    std::println("{}", type);

}
