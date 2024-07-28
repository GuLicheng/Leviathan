#include "value.hpp"

namespace toml = leviathan::toml;

// class TomlValue;
// class TomlTable;
// class TomlArray;

int main(int argc, char const *argv[])
{
    auto global = toml::make_toml<toml::string>("3.14");

    return 0;
}

