#include "toml.hpp"
#include <leviathan/print.hpp>

namespace toml = leviathan::toml;

// class TomlValue;
// class TomlTable;
// class TomlArray;

int main(int argc, char const *argv[])
{
    auto global = toml::make_toml<toml::string>("3.14");

    try
    {
        auto root = toml::loads(R"(
        
        a = true
        
        )");
    }
    catch(const std::exception& e)
    {
        Console::WriteLine(e.what());
    }
    
    Console::WriteLine("Over");

    return 0;
}

