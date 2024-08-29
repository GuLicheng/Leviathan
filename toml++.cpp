#include "toml++.hpp"
#include <iostream>

#define TOML_EXCEPTIONS 0 // only necessary if you've left them enabled in your compiler

int main()
{
    toml::parse_result result = toml::parse_file("a.toml");
    std::cout << result; // 'steal' the table from the result
    return 0;
}
