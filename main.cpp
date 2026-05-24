#include <memory>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/annotations/all.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/enum.hpp>
#include <leviathan/extc++/variant.hpp>
#include <print>


int main()
{
    std::variant<int, std::string> v = "Hello, World!";
    println("Variant holds: {}", v);

    return 0;
}