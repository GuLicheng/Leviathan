#include <variant>
#include <print>
#include <vector>
#include <algorithm>
#include <ranges>
#include <string>
#include <iostream>
#include <functional>
#include <leviathan/print.hpp>

#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/value_cast.hpp>

namespace toml = cpp::toml;
namespace json = cpp::json;


int main(int argc, char const *argv[])
{

    auto tv = toml::load(R"(D:\Library\Leviathan\test.toml)");
    auto jv = cpp::cast<json::value>(tv);
    std::println("json = {:4}", jv);

    std::println("Test passed.");

    return 0;
}

