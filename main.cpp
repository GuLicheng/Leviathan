#include <cassert>
#include <format>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <concepts>
#include <vector>
#include <set>
#include <leviathan/config_parser/json.hpp>
#include <leviathan/config_parser/toml.hpp>

int main() {

    // const char* path = R"(D:\Library\Leviathan\a.json)";

    // auto root = leviathan::json::parse_json(path);

    const char* path = R"(D:\Library\Leviathan\a.toml)";

    auto root = leviathan::toml::parse_toml(path);
    
    std::cout << std::format("Json = \n{}", root);

    return 0;
}