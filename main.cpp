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

    cpp::tuple<int, double> t(42, 3.14);
    std::println("Tuple: ({}, {})", t._0, t._1);

    std::println("Hello, Leviathan Annotations!");

    return 0;
}
