#include <leviathan/config_parser/toml.hpp>
#include <leviathan/config_parser/json.hpp>
#include <leviathan/config_parser/convert.hpp>

namespace toml = leviathan::config::toml;
namespace json = leviathan::config::json;

#include <limits>

int main(int argc, char const *argv[])
{

    std::cout << std::numeric_limits<size_t>::max();

    // const char* path = R"(D:\Library\Leviathan\a.toml)";

    // try
    // {
    //     auto root = toml::parse_toml(path);

    //     auto json = leviathan::config::toml2json(root);

    //     json::detail::json_serialize(std::cout, json, 0);
    // }
    // catch(const std::exception& e)
    // {
    //     std::cerr << e.what() << '\n';
    // }

    // std::cout << "OK\n";

    return 0;
}


// void test()
// {
//     leviathan::config::detail::convert_helper helper;

//     json::json_value boolean = helper(toml::toml_boolean(true));

//     toml::toml_table tb1, tb2;

//     tb1.emplace(toml::toml_string("here"), true);

//     tb2.emplace(toml::toml_string("Boris"), tb1);

//     json::json_value r = leviathan::config::toml2json(tb2);

//     json::detail::json_serialize(std::cout, r, 0);
// }