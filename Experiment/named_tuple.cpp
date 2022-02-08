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


using Id_field = field<"id", int>;
using Sex_field = field<"sex", bool>;
using Name_field = field<"name", std::string, []{ return "John"; }>;

using person = named_tuple<
        field<"id", int, required>,
        field<"sex", bool>,
        field<"name", std::string, []{ return "John"; }>
    >;

namespace {
    template <fixed_string S> 
    constexpr arg_type<S> operator ""_arg() { return {}; }
}

void test3()
{
    constexpr fixed_string s1 = "Hello";
    constexpr fixed_string s2 = "World";
    static_assert(s1 != s2);
    static_assert(s1 < s2);
    static_assert(find_first_fixed_string<s1, s1, s2>::value == 0);
    static_assert(find_first_fixed_string<s1, s2, s1>::value == 1);
    static_assert(find_first_fixed_string<s1, s2>::value == -1);
    static_assert(find_first_fixed_string<s1>::value == -1);
    std::cout << '(' << s1 << ")\n";
}

void test1()
{
    person p1 { arg<"id"> = 0, arg<"name"> = "Alice", arg<"sex"> = std::optional<bool>(true) };
    person p2 { "id"_arg = 1 };
    std::cout << p1 << '\n';
    std::cout << p2 << '\n';
}

void test2()
{
    using T = named_tuple<field<"int", Int32>, field<"name", double>>;
    T t{ arg<"int"> = 1 };
    std::cout << Int32::move_constructor << '\n';
    std::cout << Int32::copy_constructor << '\n';
    std::cout << Int32::total_construct() << '\n';
    std::cout << t << '\n';
}

int main()
{
    test1();
    test2();
    test3();
}





