#include <iostream>
#include "named_tuple.hpp"
#include <tuple>
#include <string_view>
#include <lv_cpp/meta/concepts.hpp>
#include <lv_cpp/string/fixed_string.hpp>
#include <type_traits>
#include <string>
#include <stdint.h>
#include <algorithm>
#include <array>
#include <optional>
#include <lv_cpp/utils/struct.hpp>

enum Gender { Male, Female, Unknown };
std::ostream& operator<<(std::ostream& os, Gender gender)
{
    using enum Gender;
    switch (gender)
    {
        case Male: os << "Male"; break;
        case Female: os << "Female"; break;
        default: os << "Unknown"; break;
    }
    return os;
}


using KeyField = field<"id", int, required_initializer>;
using OptionalField = field<"sex", Gender, []{ return Gender::Unknown; }>;
using NameFiled = field<"name", std::string, []{ return "Ada"; }>;
using RefFiled = field<"ref", double&>;

using person = named_tuple<
        KeyField,
        OptionalField,
        NameFiled,
        RefFiled
    >;

static_assert(leviathan::meta::tuple_like<person>); 
static double pi = 3.14;
void test1()
{
    person p1 { arg<"id"> = 1, arg<"ref"> = std::ref(pi), arg<"name"> = "Alice", arg<"sex"> = Gender::Female };
    person p2 { "id"_arg = 2, "ref"_arg = std::ref(pi) };
    std::cout << p1 << '\n';
    std::cout << p2 << '\n';

    // std::apply([](auto...) { }, p1); https://stackoverflow.com/questions/69216934/unable-to-use-stdapply-on-user-defined-types

}


int main()
{

    test1();
}





