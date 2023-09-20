// #include <leviathan/config_parser/json.hpp>
// #include <leviathan/config_parser/toml.hpp>
// #include <leviathan/config_parser/convert.hpp>

// #include <iostream>
// #include <memory_resource>

// int main(int argc, char const *argv[])
// {
//     const char* filename = R"(D:\Library\Leviathan\a.toml)";

//     auto toml_root = leviathan::toml::parse_toml(filename);

//     auto json_root = leviathan::toml2json(toml_root);

//     std::cout << leviathan::json::dump(json_root) << '\n';

//     // std::pmr::monotonic_buffer_resource source;

//     // auto s = source.allocate(1024 * 1024 * 2, 1);

//     std::cout << "Ok\n";

//     return 0;
// }

#include <iostream>
#include <format>
#include <leviathan/collections/internal/ring_buffer.hpp>

int main(int argc, char const *argv[])
{
    
    leviathan::collections::ring_buffer<int> buffer;

    buffer.reserve(100);

    buffer.emplace_back(0);
    buffer.emplace_back(1);
    buffer.emplace_back(2);
    buffer.emplace_back(3);

    buffer.pop_back();

    buffer.shrink_to_fit();

    buffer.show();

    std::cout << std::format("Front = {} and back = {} and capacity = {}\n", buffer.front(), buffer.back(), buffer.capacity());

    std::cout << "Ok\n";

    return 0;
}





