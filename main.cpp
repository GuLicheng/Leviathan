#include <leviathan/print.hpp>
#include <leviathan/config_parser/value_cast.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/formatter.hpp>

using namespace leviathan;

int main(int argc, char const *argv[])
{

    auto tv = toml::load("../a.toml");
    auto jv = config::toml2json::operator()(tv);
    Console::WriteLine(tv);
    Console::WriteLine(jv);
    // std::format()
    return 0;
}

