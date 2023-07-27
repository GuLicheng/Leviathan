#include <leviathan/config_parser/json.hpp>
#include <leviathan/config_parser/toml.hpp>
#include <leviathan/config_parser/convert.hpp>

int main(int argc, char const *argv[])
{
    const char* filename = R"(D:\Library\Leviathan\a.toml)";

    auto toml_root = leviathan::toml::parse_toml(filename);

    auto json_root = leviathan::toml2json(toml_root);

    std::cout << leviathan::json::dump(json_root) << '\n';

    const char* s1 = "-0.1";
    const char* s2 = "inf";
    const char* s3 = "nan";

    auto print = [](const char* str) {
        std::string_view sv = str;
        auto op = leviathan::config::from_chars_to_optional<double>(sv.begin(), sv.end());
        if (!op)
        {
            std::cout << "not a double\n";
        }
        else
        {
            std::cout << *op << '\n';
        }
    };

    print(s1);
    print(s2);
    print(s3);

    return 0;
}

