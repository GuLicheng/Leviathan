#include <leviathan/config_parser/json.hpp>


#include <iostream>
#include <functional>

namespace json = leviathan::config::json;


int main(int argc, char const *argv[])
{
	system("chcp 65001");

    json::json_value value1 = json::make(3);
    json::json_value value2 = json::make(true);
    json::json_value value3 = json::make("HelloWorld");
    json::json_value value4 = json::make(json::json_array());
    json::json_value value5 = json::make(json::json_object());
    json::json_value value6 = json::make(json::error_code::uninitialized);

    const char* path = R"(D:\Library\Leviathan\a.json)";

    auto value = json::parse_json(path);

    if (!value)
    {
        std::cout << json::report_error(value.cast_unchecked<json::error_code>());
    }
    else
    {
        json::detail::json_serialize(std::cout, value, 0);
    }

    std::cout << "\nOk\n";

    // json::detail::json_serialize(std::cout, value["BORIS", "Cost"], 0);
    // json::detail::json_serialize(std::cout, value["BORIS", "Name"], 0);
    // json::detail::json_serialize(std::cout, value["BORIS", "Hero"], 0);

    return 0;
}