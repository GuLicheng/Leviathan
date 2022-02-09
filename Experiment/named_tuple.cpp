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
#include "nt.hpp"


using Id_field = field<"id", int>;
using Sex_field = field<"sex", bool>;
using Name_field = field<"name", std::string, []{ return "John"; }>;

using person = named_tuple<
        field<"id", int, required_initializer>,
        field<"sex", bool>,
        field<"name", std::string, []{ return "John"; }>
    >;

static_assert(leviathan::meta::tuple_like<person>); 

void test1()
{
    person p1 { arg<"id"> = 0, arg<"name"> = "Alice", arg<"sex"> = std::optional<bool>(true) };
    person p2 { "id"_arg = 1 };
    std::cout << p1 << '\n';
    std::cout << p2 << '\n';
}

void test2()
{
    double pi = 3.14;
    using T = named_tuple<field<"int", Int32>, field<"name", double&>>;
    T t{ arg<"int"> = 1, arg<"name"> = std::ref(pi) };
    std::cout << Int32::move_constructor << '\n';
    std::cout << Int32::copy_constructor << '\n';
    std::cout << Int32::total_construct() << '\n';
    t.get_with<"name">() = 2.17;
    std::cout << t << '\n';
}

int main()
{
    test1();
    test2();
}





