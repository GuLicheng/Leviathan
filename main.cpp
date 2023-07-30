#include <leviathan/config_parser/json.hpp>
#include <leviathan/config_parser/toml.hpp>
#include <leviathan/config_parser/convert.hpp>

int main(int argc, char const *argv[])
{
    const char* filename = R"(D:\Library\Leviathan\a.toml)";

    auto toml_root = leviathan::toml::parse_toml(filename);

    auto json_root = leviathan::toml2json(toml_root);

    std::cout << leviathan::json::dump(json_root) << '\n';


    return 0;
}

