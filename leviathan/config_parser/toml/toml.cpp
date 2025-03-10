#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/value_cast.hpp>

namespace toml = leviathan::config::toml;
namespace json = leviathan::config::json;

int main(int argc, char const *argv[])
{
    system("chcp 65001");
    
    auto root1 = json::load(R"(D:\Library\Leviathan\salary.json)");
    auto root2 = leviathan::config::json2toml()(root1);

    std::cout << toml::formatter()(root2);

    return 0;
}
