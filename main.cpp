#include <winnow/winnow.hpp>
#include <leviathan/extc++/all.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <meta>

int main()
{
    auto json_null = cpp::json::make(nullptr);
    auto json_boolean = cpp::json::make(true);

    auto r = json_null == json_boolean;

}
