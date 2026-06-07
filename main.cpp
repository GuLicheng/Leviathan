#include <meta>
#include <format>
#include <string>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <leviathan/annotations/all.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/tuple.hpp>
#include "nlohmann.hpp"


int main(int argc, char const *argv[]) 
{

    auto t = cpp::make_tuple(42, "Hello", true, 3.14, std::vector<int>{1, 2, 3});
    std::println("Tuple: ({}, {}, {}, {}, {})", t._0, t._1, t._2, t._3, t._4);

    std::println("Hello, Leviathan Annotations!");

    return 0;
}
