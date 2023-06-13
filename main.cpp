#include <iostream>
#include <leviathan/config_parser/json.hpp>

namespace json = leviathan::config::json;

int main(int argc, char const *argv[])
{
    json::json_value value1 = json::make(3);
    json::json_value value2 = json::make(true);
    json::json_value value3 = json::make("HelloWorld");

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

    return 0;
}


 


