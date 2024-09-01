#include "value.hpp"
#include "encoder.hpp"
#include "toml.hpp"
#include <leviathan/print.hpp>

using namespace leviathan;

int main(int argc, char const *argv[])
{
    auto tv = toml::load("../a.toml");

    auto s = toml::dump(tv);
    // std::ranges::to<std::string>()
    Console::WriteLine("{}\n", s, "OK");
    return 0;
}

