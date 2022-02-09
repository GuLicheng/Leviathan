#include <tuple>
#include <iostream>
#include <string_view>
#include <lv_cpp/meta/concepts.hpp>
#include <lv_cpp/string/fixed_string.hpp>
#include <type_traits>
#include <string>
#include <stdint.h>
#include <algorithm>
#include <optional>
#include <lv_cpp/utils/struct.hpp>
#include "named_tuple.hpp"

using person = named_tuple<
        field<"id", int, required_initializer>,
        field<"sex", bool>,
        field<"name", std::string, []{ return "John"; }>,
        field<"ref", double&, required_initializer>
    >;

static_assert(leviathan::meta::tuple_like<person>); 
static double pi = 3.14;
void test1()
{
    person p1 { arg<"id"> = 0, arg<"ref"> = std::ref(pi), arg<"name"> = "Alice", arg<"sex"> = std::optional<bool>(true) };
    person p2 { "id"_arg = 1, "ref"_arg = std::ref(pi) };
    p1.get_with<"id">() = 5;
    std::cout << p1 << '\n';
    std::cout << p2 << '\n';
}

int main()
{
    test1();
}





