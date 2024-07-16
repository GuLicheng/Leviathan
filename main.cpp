#include <leviathan/value.hpp>
#include <utility>
#include <leviathan/print.hpp>
#include <leviathan/config_parser/json2.hpp>

namespace json = leviathan::config::json;

int main(int argc, char const *argv[])
{
    system("chcp 65001");

    auto root = json::load("D:\\Library\\Leviathan\\salary.json");

    if (!root)
    {
        Console::WriteLine(json::report_error(root.ec()));
        return -1;
    }
    Console::WriteLine(root);

    return 0;
}
