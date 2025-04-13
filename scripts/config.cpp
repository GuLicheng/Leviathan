#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/cmd/command.hpp>
#include <leviathan/config_parser/value_cast.hpp>

void JsonToToml(const char* source, const char* target)
{
    auto jv = cpp::json::load(source);
    auto tv = cpp::config::json2toml()(jv);
    
    std::ofstream ofs(target, std::ios::binary | std::ios::out | std::ios::app);

    if (!ofs)
    {
        std::cerr << "Unknown\n";
    }

    ofs << cpp::toml::formatter()(tv);
}

int main(int argc, char const *argv[])
{
    cpp::cmd::commandline cmds(argc, argv);

    cmds.check_size(2, true, "Usage: config <json|toml> <file>");

    const char* source = cmds[0].data();
    const char* target = cmds[1].data();

    JsonToToml(source, target);
}

