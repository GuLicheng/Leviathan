#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/value_cast.hpp>

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: config <json|toml> <file>" << std::endl;
        return 1;
    }

    const char* source = argv[1];
    const char* target = argv[2];

    auto jv = cpp::json::load(source);
    auto tv = cpp::config::json2toml()(jv);
    
    std::ofstream ofs(target, std::ios::binary | std::ios::out | std::ios::app);

    if (!ofs)
    {
        std::cerr << "Unknown\n";
    }

    ofs << cpp::toml::formatter()(tv);
}

