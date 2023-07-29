#include <leviathan/config_parser/json.hpp>
#include <leviathan/config_parser/toml.hpp>
#include <leviathan/config_parser/convert.hpp>

int main(int argc, char const *argv[])
{
    const char* filename = R"(D:\Library\Leviathan\a.toml)";

    auto toml_root = leviathan::toml::parse_toml(filename);

    auto json_root = leviathan::toml2json(toml_root);

    std::cout << leviathan::json::dump(json_root) << '\n';

    auto& table = toml_root.as_table();

    auto check = [&](const char* key, leviathan::toml::toml_integer value) {
        auto it = table.find(key);
        assert(it != table.end());
        std::cout << it->second.index() << '\n';
        assert(it->second.as_integer() == value);
    };

    check("int1", 99);
    check("int2", 42);
    check("int3", 0);
    check("int4", -17);
    check("int5", 1000);
    check("int6", 5349221);
    check("int7", 5349221);
    check("int8", 12345);

    return 0;
}

