#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/value_cast.hpp>

namespace toml = cpp::config::toml;
namespace json = cpp::config::json;

int main(int argc, char const *argv[])
{
    system("chcp 65001");
    
    auto root1 = json::load(R"(D:\Library\Leviathan\salary.json)");
    auto root2 = cpp::config::json2toml()(root1);

    // std::cout << toml::formatter()(root2);

    std::ofstream os(R"(D:\Library\Leviathan\salary.toml)", std::ios::binary | std::ios::out);

    if (os.is_open())
    {
        os << toml::formatter()(root2);
    }

    return 0;
}
