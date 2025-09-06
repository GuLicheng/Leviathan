#include <leviathan/nom/nom.hpp>
#include <leviathan/meta/type.hpp>
#include <print>

int main(int argc, char const *argv[])
{

    auto parser = nom::combinator::map(
        nom::combinator::replace_error_code(nom::character::digit1, 1),
        std::ranges::size
    );

    auto r = parser(cpp::config::context("12345"));

    using T = decltype(r);

    std::println("Type: {}", cpp::meta::name_of<T>);

    return 0;
}
